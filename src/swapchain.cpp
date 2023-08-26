#include "common.hpp"
#include "graphics.hpp"

#include "swapchain.hpp"

namespace vulkan_scene
{

using kirho::result_t;

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
            const auto result = create_image_view(p_device, p_image, p_format);
            if (result.is_error(latest_failure))
            {
                return VK_NULL_HANDLE;
            }

            return result.unwrap();
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

} // namespace vulkan_scene
