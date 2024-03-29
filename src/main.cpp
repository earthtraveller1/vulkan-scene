#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <limits>
#include <span>
#include <string_view>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "device.hpp"
#include "graphics.hpp"
#include "swapchain.hpp"
#include "window.hpp"

namespace
{

struct uniform_buffer_t
{
    glm::mat4 view;
    glm::mat4 projection;
};

struct push_constants_t
{
    glm::mat4 model;
};

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

auto create_set_layout(
    VkDevice p_device,
    std::span<const VkDescriptorSetLayoutBinding> p_layout_bindings,
    VkDescriptorType p_type,
    VkShaderStageFlags p_stage_flags
) -> kirho::result_t<VkDescriptorSetLayout, VkResult>
{
    using result_t = kirho::result_t<VkDescriptorSetLayout, VkResult>;

    const VkDescriptorSetLayoutCreateInfo set_layout_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(p_layout_bindings.size()),
        .pBindings = p_layout_bindings.data(),
    };

    VkDescriptorSetLayout set_layout;
    const auto result = vkCreateDescriptorSetLayout(
        p_device, &set_layout_info, nullptr, &set_layout
    );
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create a descriptor set layout. Vulkan error ", result,
            '.'
        );
        return result_t::error(result);
    }

    return result_t::success(set_layout);
}

auto create_descriptor_pool(
    VkDevice p_device,
    std::span<const VkDescriptorPoolSize> p_pool_sizes,
    uint32_t p_max_set_count
) noexcept -> kirho::result_t<VkDescriptorPool, VkResult>
{
    using result_t = kirho::result_t<VkDescriptorPool, VkResult>;

    const VkDescriptorPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = p_max_set_count,
        .poolSizeCount = static_cast<uint32_t>(p_pool_sizes.size()),
        .pPoolSizes = p_pool_sizes.data(),
    };

    VkDescriptorPool pool;
    const auto result =
        vkCreateDescriptorPool(p_device, &pool_info, nullptr, &pool);
    if (result != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create a descriptor pool. Vulkan error ", result, '.'
        );
        return result_t::error(result);
    }

    return result_t::success(pool);
}

auto create_descriptor_set(
    VkDevice p_device, VkDescriptorPool p_pool, VkDescriptorSetLayout p_layout
) noexcept -> kirho::result_t<VkDescriptorSet, VkResult>
{
    using result_t = kirho::result_t<VkDescriptorSet, VkResult>;
    using vulkan_scene::print_error;

    const VkDescriptorSetAllocateInfo set_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = p_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &p_layout,
    };

    VkDescriptorSet set;
    const auto result = vkAllocateDescriptorSets(p_device, &set_info, &set);
    if (result != VK_SUCCESS)
    {
        print_error(
            "Failed to allocate a descriptor set. Vulkan error ", result
        );
        return result_t::error(result);
    }

    return result_t::success(set);
}

struct device_t
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device;
    uint32_t graphics_queue_family;
    uint32_t present_queue_family;

    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;

    device_t(
        VkInstance p_instance,
        VkDebugUtilsMessengerEXT p_debug_messenger,
        VkSurfaceKHR p_surface,
        VkPhysicalDevice p_physical_device,
        uint32_t p_graphics_queue_family,
        uint32_t p_present_queue_family,
        VkDevice p_device,
        VkQueue p_graphics_queue,
        VkQueue p_present_queue
    )
        : instance(p_instance), debug_messenger(p_debug_messenger),
          surface(p_surface), physical_device(p_physical_device),
          graphics_queue_family(p_graphics_queue_family),
          present_queue_family(p_present_queue_family), device(p_device),
          graphics_queue(p_graphics_queue), present_queue(p_present_queue)
    {
    }

    static auto create(GLFWwindow* window, bool enable_validation) -> device_t
    {
        const auto instance =
            vulkan_scene::create_vulkan_instance(enable_validation).unwrap();

        const auto debug_messenger =
            enable_validation
                ? vulkan_scene::create_debug_messenger(instance).unwrap()
                : VK_NULL_HANDLE;

        const auto surface =
            vulkan_scene::create_surface(instance, window).unwrap();

        const auto
            [physical_device, graphics_queue_family, present_queue_family] =
                vulkan_scene::choose_physical_device(instance, surface)
                    .unwrap();

        const auto [device, graphics_queue, present_queue] =
            vulkan_scene::create_logical_device(
                physical_device, graphics_queue_family, present_queue_family
            )
                .unwrap();

        return device_t{
            instance,        debug_messenger,       surface,
            physical_device, graphics_queue_family, present_queue_family,
            device,          graphics_queue,        present_queue,
        };
    }

    device_t(const device_t&) = delete;
    device_t& operator=(const device_t&) = delete;

    operator VkDevice() const
    {
        return device;
    }

    ~device_t()
    {
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);

        if (debug_messenger != VK_NULL_HANDLE)
        {
            vulkan_scene::destroy_debug_messenger(instance, debug_messenger);
        }

        vkDestroyInstance(instance, nullptr);
    }
};

enum class axis_t
{
    X,
    Y,
    Z
};

auto append_cube_face_to_mesh(
    axis_t p_axis,
    bool p_negate,
    bool p_backface,
    std::vector<vulkan_scene::vertex_t>& p_vertices,
    std::vector<uint16_t>& p_indices
) -> void
{
    const float values[][2]{
        {0.5f, -0.5f},
        {0.5f, 0.5f},
        {-0.5f, 0.5f},
        {-0.5f, -0.5f},
    };

    const float third_value = p_negate ? -0.5f : 0.5f;
    const float normal_third_value = p_negate ? -1.0f : 1.0f;

    const std::array<uint16_t, 6> indices_back{3, 2, 1, 1, 0, 3};
    const std::array<uint16_t, 6> indices_front{3, 0, 1, 1, 2, 3};

    const float texture_coordinates[][2]{
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.0f, 0.0f},
    };

    const auto pivot_index = static_cast<uint16_t>(p_vertices.size());

    for (int i = 0; i < 4; i++)
    {
        float x_value, y_value, z_value;
        float normal_x = 0.0f, normal_y = 0.0f, normal_z = 0.0f;

        switch (p_axis)
        {
        case axis_t::X:
            z_value = values[i][0];
            y_value = values[i][1];
            x_value = third_value;
            normal_x = normal_third_value;
            break;
        case axis_t::Y:
            x_value = values[i][0];
            z_value = values[i][1];
            y_value = third_value;
            normal_y = normal_third_value;
            break;
        case axis_t::Z:
            x_value = values[i][0];
            y_value = values[i][1];
            z_value = third_value;
            normal_z = normal_third_value;
        }

        p_vertices.push_back({
            .position = {x_value, y_value, z_value},
            .uv = {texture_coordinates[i][0], texture_coordinates[i][1]},
            .normal = {normal_x, normal_y, normal_z},
        });
    }

    const auto& indices = p_backface ? indices_back : indices_front;

    for (auto index : indices)
    {
        p_indices.push_back(pivot_index + index);
    }
}

} // namespace

auto main(int argc, char** argv) noexcept -> int
{
    auto enable_validation = false;

    for (const char* const* arg = argv; arg < argv + argc; arg++)
    {
        if (std::strcmp(*arg, "--enable-validation") == 0)
        {
            enable_validation = true;
        }
    }

    const auto window =
        vulkan_scene::create_window("Vulkan Scene", WINDOW_WIDTH, WINDOW_HEIGHT)
            .unwrap();

    const auto device = device_t::create(window, enable_validation);

    const auto command_pool =
        vulkan_scene::create_command_pool(device, device.graphics_queue_family)
            .unwrap();

    const auto main_command_buffer =
        vulkan_scene::create_command_buffer(device, command_pool).unwrap();

    const auto fence = vulkan_scene::create_fence(device).unwrap();

    const auto image_available_semaphore =
        vulkan_scene::create_semaphore(device).unwrap();

    const auto render_done_semaphore =
        vulkan_scene::create_semaphore(device).unwrap();

    auto swapchain =
        vulkan_scene::create_swapchain(
            device, device.physical_device, device.graphics_queue_family,
            device.present_queue_family, window, device.surface
        )
            .unwrap();

    auto swapchain_image_views = vulkan_scene::create_image_views(
                                     device, swapchain.images, swapchain.format
    )
                                     .unwrap();

    const auto render_pass =
        vulkan_scene::create_render_pass(device, swapchain.format).unwrap();

    auto framebuffers =
        vulkan_scene::create_framebuffers(
            device, swapchain_image_views, swapchain.extent, render_pass
        )
            .unwrap();

    const auto vertex_shader_module =
        vulkan_scene::create_shader_module(device, "shaders/basic.vert.spv")
            .unwrap();

    const auto fragment_shader_module =
        vulkan_scene::create_shader_module(device, "shaders/basic.frag.spv")
            .unwrap();

    const auto descriptor_set_layout =
        create_set_layout(
            device,
            std::array{
                VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr,
                },
                VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
                },
            },
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT
        )
            .unwrap();

    const auto push_constant_range = VkPushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(push_constants_t),
    };

    const auto pipeline_layout = vulkan_scene::create_pipeline_layout(
                                     device, std::array{descriptor_set_layout},
                                     std::array{push_constant_range}
    )
                                     .unwrap();

    const auto graphics_pipeline =
        vulkan_scene::create_graphics_pipeline(
            device, render_pass, pipeline_layout, vertex_shader_module,
            fragment_shader_module
        )
            .unwrap();

    uniform_buffer_t uniform_buffer_data{};

    const auto uniform_buffer =
        vulkan_scene::create_uniform_buffer(
            device.physical_device, device, &uniform_buffer_data,
            sizeof(uniform_buffer_data)
        )
            .unwrap();

    constexpr std::array descriptor_pool_sizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
        },
    };

    const auto descriptor_pool =
        create_descriptor_pool(device, descriptor_pool_sizes, 1).unwrap();

    const auto descriptor_set =
        create_descriptor_set(device, descriptor_pool, descriptor_set_layout)
            .unwrap();

#if 0
    const auto indices = std::array<uint16_t, 36>{
        // clang-format off
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        7, 2, 6, 7, 3, 2,
        4, 5, 1, 4, 1, 0,
        5, 2, 1, 2, 5, 6,
        4, 0, 3, 4, 3, 7,
        // clang-format on
    };

    const auto vertices = std::array{
        vulkan_scene::vertex_t{
            .position = {0.5f, -0.5f, 0.5f},
            .uv = {1.0f, 0.0f},
        },
        vulkan_scene::vertex_t{
            .position = {0.5f, 0.5f, 0.5f},
            .uv = {1.0f, 1.0f},
        },
        vulkan_scene::vertex_t{
            .position = {-0.5f, 0.5f, 0.5f},
            .uv = {0.0f, 1.0f},
        },
        vulkan_scene::vertex_t{
            .position = {-0.5f, -0.5f, 0.5f},
            .uv = {0.0f, 0.0f},
        },
        vulkan_scene::vertex_t{
            .position = {0.5f, -0.5f, -0.5f},
            .uv = {0.0f, 1.0f},
        },
        vulkan_scene::vertex_t{
            .position = {0.5f, 0.5f, -0.5f},
            .uv = {0.0f, 0.0f},
        },
        vulkan_scene::vertex_t{
            .position = {-0.5f, 0.5f, -0.5f},
            .uv = {1.0f, 0.0f},
        },
        vulkan_scene::vertex_t{
            .position = {-0.5f, -0.5f, -0.5f},
            .uv = {1.0f, 1.0f},
        },
    };
#else
    std::vector<vulkan_scene::vertex_t> vertices;
    std::vector<uint16_t> indices;

    append_cube_face_to_mesh(axis_t::Z, false, false, vertices, indices);
    append_cube_face_to_mesh(axis_t::Z, true, true, vertices, indices);
    append_cube_face_to_mesh(axis_t::X, true, false, vertices, indices);
    append_cube_face_to_mesh(axis_t::X, false, true, vertices, indices);
    append_cube_face_to_mesh(axis_t::Y, true, false, vertices, indices);
    append_cube_face_to_mesh(axis_t::Y, false, true, vertices, indices);
#endif

    const auto vertex_buffer =
        vulkan_scene::create_buffer(
            device.physical_device, device, device.graphics_queue, command_pool,
            vulkan_scene::buffer_type_t::VERTEX, vertices.data(),
            vertices.size() * sizeof(vulkan_scene::vertex_t)
        )
            .unwrap();

    const auto index_buffer =
        vulkan_scene::create_buffer(
            device.physical_device, device, device.graphics_queue, command_pool,
            vulkan_scene::buffer_type_t::INDEX, indices.data(),
            sizeof(uint16_t) * indices.size()
        )
            .unwrap();

    const auto image =
        vulkan_scene::create_image(
            device.physical_device, device, device.graphics_queue, command_pool,
            "textures/can-pooper.png"
        )
            .unwrap();

    const auto sampler = vulkan_scene::create_sampler(
                             device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, false
    )
                             .unwrap();

    {
        const VkDescriptorBufferInfo uniform_buffer_info{
            .buffer = uniform_buffer.buffer,
            .offset = 0,
            .range = sizeof(uniform_buffer_data),
        };

        const VkDescriptorImageInfo image_info{
            .sampler = sampler,
            .imageView = image.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        const VkWriteDescriptorSet set_write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = &image_info,
            .pBufferInfo = &uniform_buffer_info,
            .pTexelBufferView = nullptr,
        };

        const VkWriteDescriptorSet set_write_2{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_set,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        const std::array set_writes{set_write, set_write_2};

        vkUpdateDescriptorSets(
            device, set_writes.size(), set_writes.data(), 0, nullptr
        );
    }

    using vulkan_scene::print_error;

    double delta_time = 0.0;

    push_constants_t push_constants{};

    double old_cursor_x, old_cursor_y;
    bool first_frame = true;

    float total_x_rotation = 0.0f, total_y_rotation = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        const double start_time = glfwGetTime();

        double cursor_x, cursor_y;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);

        if (first_frame)
        {
            old_cursor_x = cursor_x;
            old_cursor_y = cursor_y;

            first_frame = false;
        }

        VkResult result;

        uint32_t image_index;
        result = vkAcquireNextImageKHR(
            device, swapchain.swapchain, std::numeric_limits<uint64_t>::max(),
            image_available_semaphore, VK_NULL_HANDLE, &image_index
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            const auto result = vkDeviceWaitIdle(device);
            if (result != VK_SUCCESS)
            {
                print_error(
                    "Failed to wait for the device to complete all "
                    "operations. "
                    "Vulkan error ",
                    result
                );
                return EXIT_FAILURE;
            }

            std::for_each(
                framebuffers.cbegin(), framebuffers.cend(),
                [&device](VkFramebuffer fb)
                { vkDestroyFramebuffer(device, fb, nullptr); }
            );

            std::for_each(
                swapchain_image_views.cbegin(), swapchain_image_views.cend(),
                [&device](VkImageView view)
                { vkDestroyImageView(device, view, nullptr); }
            );

            vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);

            swapchain = vulkan_scene::create_swapchain(
                            device, device.physical_device,
                            device.graphics_queue_family,
                            device.present_queue_family, window, device.surface
            )
                            .unwrap();

            swapchain_image_views =
                vulkan_scene::create_image_views(
                    device, swapchain.images, swapchain.format
                )
                    .unwrap();

            framebuffers =
                vulkan_scene::create_framebuffers(
                    device, swapchain_image_views, swapchain.extent, render_pass
                )
                    .unwrap();

            continue;
        }

        vkWaitForFences(
            device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()
        );
        vkResetFences(device, 1, &fence);

        vkResetCommandBuffer(main_command_buffer, 0);

        const VkCommandBufferBeginInfo command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        result = vkBeginCommandBuffer(
            main_command_buffer, &command_buffer_begin_info
        );
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to being recording the main command buffer. Vulkan "
                "error ",
                result
            );
            return EXIT_FAILURE;
        }

        const VkClearValue clear_value{
            .color =
                VkClearColorValue{
                    .float32 =
                        {
                            0.0f,
                            0.0f,
                            0.0f,
                            1.0f,
                        },
                },
        };

        const VkRenderPassBeginInfo render_pass_begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass,
            .framebuffer = framebuffers.at(image_index),
            .renderArea =
                VkRect2D{
                    .offset =
                        VkOffset2D{
                            .x = 0,
                            .y = 0,
                        },
                    .extent = swapchain.extent,
                },
            .clearValueCount = 1,
            .pClearValues = &clear_value, // TODO
        };

        vkCmdBeginRenderPass(
            main_command_buffer, &render_pass_begin_info,
            VK_SUBPASS_CONTENTS_INLINE
        );

        vkCmdBindPipeline(
            main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphics_pipeline
        );

        const VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapchain.extent.width),
            .height = static_cast<float>(swapchain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        vkCmdSetViewport(main_command_buffer, 0, 1, &viewport);

        const VkRect2D scissor{
            .offset = VkOffset2D{.x = 0, .y = 0},
            .extent = swapchain.extent,
        };

        vkCmdSetScissor(main_command_buffer, 0, 1, &scissor);

        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(
            main_command_buffer, 0, 1, &vertex_buffer.buffer, &offset
        );

        vkCmdBindIndexBuffer(
            main_command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16
        );

        // vkCmdDraw(
        //     main_command_buffer, static_cast<uint32_t>(vertices.size()),
        //     1, 0, 0
        // );

        int screen_width, screen_height;
        glfwGetFramebufferSize(window, &screen_width, &screen_height);
        const auto aspect = static_cast<float>(screen_width) /
                            static_cast<float>(screen_height);

        uniform_buffer_data.view = glm::mat4(1.0f);
        uniform_buffer_data.view = glm::translate(
            uniform_buffer_data.view, glm::vec3(0.0f, 0.0f, -2.0f)
        );

        uniform_buffer_data.projection =
            glm::perspective(45.0f, aspect, 0.1f, 100.0f);

        uniform_buffer_t* uniform_buffer_ptr;
        vkMapMemory(
            device, uniform_buffer.memory, 0, sizeof(uniform_buffer_t), 0,
            reinterpret_cast<void**>(&uniform_buffer_ptr)
        );

        *uniform_buffer_ptr = uniform_buffer_data;

        vkUnmapMemory(device, uniform_buffer.memory);

        vkCmdBindDescriptorSets(
            main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout, 0, 1, &descriptor_set, 0, nullptr
        );

        // push_constants.color_shift = sin(glfwGetTime() * 2.0) / 2.0 +
        // 0.5;
        // push_constants.model = glm::rotate(
        //     push_constants.model,
        //     50.0f * static_cast<float>(glm::radians(glfwGetTime())),
        //     glm::vec3(0.5f, 1.0f, 0.0f)
        // );
        push_constants.model = glm::mat4(1.0);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            const double delta_cursor_x = cursor_x - old_cursor_x;
            const double delta_cursor_y = old_cursor_y - cursor_y;

            total_x_rotation +=
                static_cast<float>(glm::radians(delta_cursor_x * 2.0));
            total_y_rotation +=
                static_cast<float>(glm::radians(delta_cursor_y * 2.0));
        }

        push_constants.model = glm::rotate(
            push_constants.model,
            static_cast<float>(glm::radians(total_x_rotation)),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        push_constants.model = glm::rotate(
            push_constants.model,
            static_cast<float>(glm::radians(total_y_rotation)),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        vkCmdPushConstants(
            main_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
            sizeof(push_constants), &push_constants
        );

        vkCmdDrawIndexed(main_command_buffer, indices.size(), 1, 0, 0, 0);

        vkCmdEndRenderPass(main_command_buffer);

        result = vkEndCommandBuffer(main_command_buffer);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to stop recording the main command buffer. Vulkan "
                "error ",
                result
            );
            return EXIT_FAILURE;
        }

        VkPipelineStageFlags wait_stage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        const VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_available_semaphore,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &main_command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_done_semaphore,
        };

        result = vkQueueSubmit(device.graphics_queue, 1, &submit_info, fence);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to submit the command buffer to the graphics "
                "queue. "
                "Vulkan error ",
                result, "."
            );
            return EXIT_FAILURE;
        }

        const VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_done_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain.swapchain,
            .pImageIndices = &image_index,
            .pResults = nullptr,
        };

        result = vkQueuePresentKHR(device.present_queue, &present_info);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            const auto result = vkDeviceWaitIdle(device);
            if (result != VK_SUCCESS)
            {
                print_error(
                    "Failed to wait for the device to complete all "
                    "operations. "
                    "Vulkan error ",
                    result
                );
                return EXIT_FAILURE;
            }

            std::for_each(
                framebuffers.cbegin(), framebuffers.cend(),
                [&device](VkFramebuffer fb)
                { vkDestroyFramebuffer(device, fb, nullptr); }
            );

            std::for_each(
                swapchain_image_views.cbegin(), swapchain_image_views.cend(),
                [&device](VkImageView view)
                { vkDestroyImageView(device, view, nullptr); }
            );

            vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);

            swapchain = vulkan_scene::create_swapchain(
                            device, device.physical_device,
                            device.graphics_queue_family,
                            device.present_queue_family, window, device.surface
            )
                            .unwrap();

            swapchain_image_views =
                vulkan_scene::create_image_views(
                    device, swapchain.images, swapchain.format
                )
                    .unwrap();

            framebuffers =
                vulkan_scene::create_framebuffers(
                    device, swapchain_image_views, swapchain.extent, render_pass
                )
                    .unwrap();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            print_error(
                "Failed to present the output to the screen. Vulkan error ",
                result, "."
            );
            return EXIT_FAILURE;
        }

        glfwPollEvents();

        old_cursor_x = cursor_x;
        old_cursor_y = cursor_y;

        const double end_time = glfwGetTime();
        delta_time = end_time - start_time;
        const double framerate = 1.0 / delta_time;

        std::cout << "[INFO]: Framerate: " << framerate << "\r";
    }

    vkDeviceWaitIdle(device);

    vkDestroySampler(device, sampler, nullptr);
    vulkan_scene::destroy_image(device, image);
    vulkan_scene::destroy_buffer(device, index_buffer);
    vulkan_scene::destroy_buffer(device, vertex_buffer);
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    vulkan_scene::destroy_buffer(device, uniform_buffer);
    vkDestroyPipeline(device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    vkDestroyShaderModule(device, fragment_shader_module, nullptr);
    vkDestroyShaderModule(device, vertex_shader_module, nullptr);
    for (const auto buffer : framebuffers)
        vkDestroyFramebuffer(device, buffer, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);
    for (const auto view : swapchain_image_views)
        vkDestroyImageView(device, view, nullptr);
    vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
    vkDestroySemaphore(device, render_done_semaphore, nullptr);
    vkDestroySemaphore(device, image_available_semaphore, nullptr);
    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, command_pool, 1, &main_command_buffer);
    vkDestroyCommandPool(device, command_pool, nullptr);

    vulkan_scene::destroy_window(window);

    return 0;
}
