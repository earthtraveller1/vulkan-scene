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
#include "window.hpp"

namespace
{

using kirho::result_t;

constexpr uint16_t WINDOW_WIDTH = 1280;
constexpr uint16_t WINDOW_HEIGHT = 720;

struct swapchain_t
{
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    VkFormat format;
    VkExtent2D extent;
};

auto create_swapchain(
    VkDevice p_device,
    VkPhysicalDevice p_physical_device,
    uint32_t p_graphics_family,
    uint32_t p_present_family,
    GLFWwindow* p_window,
    VkSurfaceKHR p_surface
) noexcept -> result_t<swapchain_t, VkResult>
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        p_physical_device, p_surface, &surface_capabilities
    );

    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        p_physical_device, p_surface, &surface_format_count, nullptr
    );

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        p_physical_device, p_surface, &surface_format_count,
        surface_formats.data()
    );

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        p_physical_device, p_surface, &present_mode_count, nullptr
    );

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        p_physical_device, p_surface, &present_mode_count, present_modes.data()
    );

    VkSurfaceFormatKHR surface_format = surface_formats.front();

    for (const auto& format : surface_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = format;
            break;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (auto mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
            break;
        }
    }

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(p_window, &framebuffer_width, &framebuffer_height);

    const VkExtent2D swap_extent =
        surface_capabilities.currentExtent.width !=
                std::numeric_limits<uint32_t>::max()
            ? surface_capabilities.currentExtent
            : VkExtent2D{
                  std::clamp(
                      static_cast<uint32_t>(framebuffer_width),
                      surface_capabilities.minImageExtent.width,
                      surface_capabilities.maxImageExtent.width
                  ),
                  std::clamp(
                      static_cast<uint32_t>(framebuffer_height),
                      surface_capabilities.minImageExtent.height,
                      surface_capabilities.maxImageExtent.height
                  ),
              };

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    const auto has_max_image_count = surface_capabilities.maxImageCount > 0;
    if (has_max_image_count && image_count > surface_capabilities.maxImageCount)
    {
        image_count = surface_capabilities.maxImageCount;
    }

    const auto queue_families_same = p_graphics_family == p_present_family;

    std::array<uint32_t, 2> queue_families{p_graphics_family, p_present_family};

    const VkSwapchainCreateInfoKHR swapchain_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swap_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = queue_families_same ? VK_SHARING_MODE_EXCLUSIVE
                                                : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount =
            queue_families_same ? 0
                                : static_cast<uint32_t>(queue_families.size()),
        .pQueueFamilyIndices =
            queue_families_same ? nullptr : queue_families.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR swapchain;

    using result_t_t = result_t<swapchain_t, VkResult>;

    const auto result_t =
        vkCreateSwapchainKHR(p_device, &swapchain_info, nullptr, &swapchain);
    if (result_t != VK_SUCCESS)
    {
        vulkan_scene::print_error(
            "Failed to create the swapchain. Vulkan error ", result_t
        );
        return result_t_t::error(result_t);
    }

    vkGetSwapchainImagesKHR(p_device, swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(p_device, swapchain, &image_count, images.data());

    return result_t_t::success(swapchain_t{
        swapchain, images, surface_format.format, swap_extent});
}

auto create_image_views(
    VkDevice p_device, const std::vector<VkImage>& p_images, VkFormat p_format
) noexcept -> result_t<std::vector<VkImageView>, VkResult>
{
    using result_t_t = result_t<std::vector<VkImageView>, VkResult>;

    std::vector<VkImageView> image_views(p_images.size());

    VkResult latest_failure = VK_SUCCESS;

    std::transform(
        p_images.cbegin(), p_images.cend(), image_views.begin(),
        [p_device, p_format, &latest_failure](VkImage p_image) -> VkImageView
        {
            const VkImageViewCreateInfo view_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = p_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = p_format,
                .components =
                    VkComponentMapping{
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                .subresourceRange =
                    {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
            };

            VkImageView image_view;
            const auto result =
                vkCreateImageView(p_device, &view_info, nullptr, &image_view);
            if (result != VK_SUCCESS)
            {
                latest_failure = result;
                vulkan_scene::print_error(
                    "Failed to create an image view. Vulkan error ", result, '.'
                );
            }

            return image_view;
        }
    );

    if (latest_failure != VK_SUCCESS)
    {
        return result_t_t::error(latest_failure);
    }

    return result_t_t::success(image_views);
}

auto create_framebuffers(
    VkDevice p_device,
    const std::vector<VkImageView>& p_image_views,
    const VkExtent2D& p_swapchain_extent,
    VkRenderPass p_render_pass
) noexcept -> result_t<std::vector<VkFramebuffer>, VkResult>
{
    std::vector<VkFramebuffer> framebuffers;

    using result_tt = result_t<std::vector<VkFramebuffer>, VkResult>;

    VkResult latest_failure = VK_SUCCESS;

    std::ranges::transform(
        p_image_views, std::back_inserter(framebuffers),
        [p_device, p_render_pass, &p_swapchain_extent,
         &latest_failure](VkImageView p_image_view) -> VkFramebuffer
        {
            const VkFramebufferCreateInfo framebuffer_info{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = p_render_pass,
                .attachmentCount = 1,
                .pAttachments = &p_image_view,
                .width = p_swapchain_extent.width,
                .height = p_swapchain_extent.height,
                .layers = 1,
            };

            VkFramebuffer framebuffer;
            const auto result = vkCreateFramebuffer(
                p_device, &framebuffer_info, nullptr, &framebuffer
            );
            if (result != VK_SUCCESS)
            {
                vulkan_scene::print_error(
                    "Failed to create a Vulkan framebuffer. Vulkan error ",
                    result, '.'
                );
                latest_failure = result;
            }

            return framebuffer;
        }
    );

    if (latest_failure != VK_SUCCESS)
    {
        return result_tt::error(latest_failure);
    }

    return result_tt::success(framebuffers);
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
        create_swapchain(
            logical_device, physical_device, graphics_queue_family,
            present_queue_family, window, surface
        )
            .unwrap();
    defer(
        swapchain,
        vkDestroySwapchainKHR(logical_device, swapchain.swapchain, nullptr)
    );

    const auto swapchain_image_views =
        create_image_views(logical_device, swapchain.images, swapchain.format)
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
        create_framebuffers(
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

    const auto vertices = std::array<vulkan_scene::vertex_t, 3>{
        vulkan_scene::vertex_t{.position = {0.0f, -0.5f, 0.0f}},
        vulkan_scene::vertex_t{.position = {0.5f, 0.5f, 0.0f}},
        vulkan_scene::vertex_t{.position = {-0.5f, 0.5f, 0.0f}},
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

        vkCmdDraw(
            main_command_buffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0
        );

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
