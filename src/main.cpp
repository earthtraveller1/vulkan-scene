#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <ios>
#include <limits>
#include <span>
#include <string_view>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "device.hpp"
#include "graphics.hpp"
#include "swapchain.hpp"
#include "window.hpp"

namespace
{

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

auto create_set_layout(
    VkDevice p_device,
    uint32_t p_binding,
    VkDescriptorType p_type,
    VkShaderStageFlags p_stage_flags
) -> kirho::result_t<VkDescriptorSetLayout, VkResult>
{
    using result_t = kirho::result_t<VkDescriptorSetLayout, VkResult>;

    const VkDescriptorSetLayoutBinding layout_binding{
        .binding = p_binding,
        .descriptorType = p_type,
        .descriptorCount = 1,
        .stageFlags = p_stage_flags,
        .pImmutableSamplers = nullptr,
    };

    const VkDescriptorSetLayoutCreateInfo set_layout_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &layout_binding,
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

} // namespace

auto main() noexcept -> int
{
    const auto enable_validation = true;

    const auto window =
        vulkan_scene::create_window("Vulkan Scene", WINDOW_WIDTH, WINDOW_HEIGHT)
            .unwrap();
    defer(window, vulkan_scene::destroy_window(window));

    const auto instance =
        vulkan_scene::create_vulkan_instance(enable_validation).unwrap();
    defer(instance, vkDestroyInstance(instance, nullptr));

    const auto debug_messenger =
        enable_validation
            ? vulkan_scene::create_debug_messenger(instance).unwrap()
            : VK_NULL_HANDLE;
    defer(
        debug_messenger,
        enable_validation
            ? vulkan_scene::destroy_debug_messenger(instance, debug_messenger)
            : (void)0
    );

    const auto surface =
        vulkan_scene::create_surface(instance, window).unwrap();
    defer(surface, vkDestroySurfaceKHR(instance, surface, nullptr));

    const auto [physical_device, graphics_queue_family, present_queue_family] =
        vulkan_scene::choose_physical_device(instance, surface).unwrap();

    const auto [logical_device, graphics_queue, present_queue] =
        vulkan_scene::create_logical_device(
            physical_device, graphics_queue_family, present_queue_family
        )
            .unwrap();
    defer(logical_device, vkDestroyDevice(logical_device, nullptr));

    const auto command_pool =
        vulkan_scene::create_command_pool(logical_device, graphics_queue_family)
            .unwrap();
    defer(
        command_pool,
        vkDestroyCommandPool(logical_device, command_pool, nullptr)
    );

    const auto main_command_buffer =
        vulkan_scene::create_command_buffer(logical_device, command_pool)
            .unwrap();
    defer(
        main_command_buffer,
        vkFreeCommandBuffers(
            logical_device, command_pool, 1, &main_command_buffer
        )
    );

    const auto fence = vulkan_scene::create_fence(logical_device).unwrap();
    defer(fence, vkDestroyFence(logical_device, fence, nullptr));

    const auto image_available_semaphore =
        vulkan_scene::create_semaphore(logical_device).unwrap();
    defer(
        semaphore,
        vkDestroySemaphore(logical_device, image_available_semaphore, nullptr)
    );

    const auto render_done_semaphore =
        vulkan_scene::create_semaphore(logical_device).unwrap();
    defer(
        render_done_semaphore,
        vkDestroySemaphore(logical_device, render_done_semaphore, nullptr)
    );

    const auto swapchain =
        vulkan_scene::create_swapchain(
            logical_device, physical_device, graphics_queue_family,
            present_queue_family, window, surface
        )
            .unwrap();
    defer(
        swapchain,
        vkDestroySwapchainKHR(logical_device, swapchain.swapchain, nullptr)
    );

    const auto swapchain_image_views =
        vulkan_scene::create_image_views(
            logical_device, swapchain.images, swapchain.format
        )
            .unwrap();
    defer(
        swapchain_image_views,
        std::for_each(
            swapchain_image_views.cbegin(), swapchain_image_views.cend(),
            [logical_device](VkImageView p_view)
            { vkDestroyImageView(logical_device, p_view, nullptr); }
        )
    );

    const auto render_pass =
        vulkan_scene::create_render_pass(logical_device, swapchain.format)
            .unwrap();
    defer(
        render_pass, vkDestroyRenderPass(logical_device, render_pass, nullptr)
    );

    const auto framebuffers =
        vulkan_scene::create_framebuffers(
            logical_device, swapchain_image_views, swapchain.extent, render_pass
        )
            .unwrap();
    defer(
        framebuffers,
        std::ranges::for_each(
            framebuffers, [logical_device](auto buffer)
            { vkDestroyFramebuffer(logical_device, buffer, nullptr); }
        )
    );

    const auto vertex_shader_module =
        vulkan_scene::create_shader_module(
            logical_device, "shaders/basic.vert.spv"
        )
            .unwrap();
    defer(
        vertex_shader_module,
        vkDestroyShaderModule(logical_device, vertex_shader_module, nullptr)
    );

    const auto fragment_shader_module =
        vulkan_scene::create_shader_module(
            logical_device, "shaders/basic.frag.spv"
        )
            .unwrap();
    defer(
        fragment_shader_module,
        vkDestroyShaderModule(logical_device, fragment_shader_module, nullptr)
    );

    const auto descriptor_set_layout =
        create_set_layout(
            logical_device, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT
        )
            .unwrap();
    defer(
        descriptor_set_layout,
        vkDestroyDescriptorSetLayout(
            logical_device, descriptor_set_layout, nullptr
        )
    );

    const auto pipeline_layout =
        vulkan_scene::create_pipeline_layout(logical_device).unwrap();
    defer(
        pipeline_layout,
        vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr)
    );

    const auto graphics_pipeline =
        vulkan_scene::create_graphics_pipeline(
            logical_device, render_pass, pipeline_layout, vertex_shader_module,
            fragment_shader_module
        )
            .unwrap();
    defer(
        graphics_pipeline,
        vkDestroyPipeline(logical_device, graphics_pipeline, nullptr)
    );

    constexpr std::array<VkDescriptorPoolSize, 1> descriptor_pool_sizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
        },
    };

    const auto descriptor_pool =
        create_descriptor_pool(logical_device, descriptor_pool_sizes, 1)
            .unwrap();
    defer(
        descriptor_pool,
        vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr)
    );

    const auto descriptor_set =
        create_descriptor_set(
            logical_device, descriptor_pool, descriptor_set_layout
        )
            .unwrap();
    (void)descriptor_set;

    const auto indices = std::array<uint16_t, 6>{0, 1, 2, 0, 2, 3};

    const auto vertices = std::array<vulkan_scene::vertex_t, 4>{
        vulkan_scene::vertex_t{.position = {0.5f, -0.5f, 0.0f}},
        vulkan_scene::vertex_t{.position = {0.5f, 0.5f, 0.0f}},
        vulkan_scene::vertex_t{.position = {-0.5f, 0.5f, 0.0f}},
        vulkan_scene::vertex_t{.position = {-0.5f, -0.5f, 0.0f}},
    };

    const auto vertex_buffer =
        vulkan_scene::create_buffer(
            physical_device, logical_device, graphics_queue, command_pool,
            vulkan_scene::buffer_type_t::VERTEX, vertices.data(),
            vertices.size() * sizeof(vulkan_scene::vertex_t)
        )
            .unwrap();

    defer(
        vertex_buffer,
        vulkan_scene::destroy_buffer(logical_device, vertex_buffer)
    );

    const auto index_buffer =
        vulkan_scene::create_buffer(
            physical_device, logical_device, graphics_queue, command_pool,
            vulkan_scene::buffer_type_t::INDEX, indices.data(),
            sizeof(uint16_t) * indices.size()
        )
            .unwrap();

    defer(
        index_buffer, vulkan_scene::destroy_buffer(logical_device, index_buffer)
    );

    using vulkan_scene::print_error;

    while (!glfwWindowShouldClose(window))
    {
        vkWaitForFences(
            logical_device, 1, &fence, VK_TRUE,
            std::numeric_limits<uint64_t>::max()
        );
        vkResetFences(logical_device, 1, &fence);

        uint32_t image_index;
        vkAcquireNextImageKHR(
            logical_device, swapchain.swapchain,
            std::numeric_limits<uint64_t>::max(), image_available_semaphore,
            VK_NULL_HANDLE, &image_index
        );

        vkResetCommandBuffer(main_command_buffer, 0);

        const VkCommandBufferBeginInfo command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        auto result = vkBeginCommandBuffer(
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
        //     main_command_buffer, static_cast<uint32_t>(vertices.size()), 1,
        //     0, 0
        // );

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

        result = vkQueueSubmit(graphics_queue, 1, &submit_info, fence);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to submit the command buffer to the graphics queue. "
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

        result = vkQueuePresentKHR(present_queue, &present_info);
        if (result != VK_SUCCESS)
        {
            print_error(
                "Failed to present the output to the screen. Vulkan error ",
                result, "."
            );
            return EXIT_FAILURE;
        }

        glfwPollEvents();
    }

    vkDeviceWaitIdle(logical_device);

    return 0;
}
