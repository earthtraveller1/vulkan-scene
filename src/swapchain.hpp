#pragma once

#include "window.hpp"

namespace vulkan_scene
{

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
) noexcept -> kirho::result_t<swapchain_t, VkResult>;

auto create_image_views(
    VkDevice p_device, const std::vector<VkImage>& p_images, VkFormat p_format
) noexcept -> kirho::result_t<std::vector<VkImageView>, VkResult>;

auto create_framebuffers(
    VkDevice p_device,
    const std::vector<VkImageView>& p_image_views,
    const VkExtent2D& p_swapchain_extent,
    VkRenderPass p_render_pass
) noexcept -> kirho::result_t<std::vector<VkFramebuffer>, VkResult>;

} // namespace vulkan_scene
