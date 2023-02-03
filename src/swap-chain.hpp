#pragma once

#include "framebuffer-manager.hpp"

namespace vulkan_scene
{
class Device;

// A basic class that can encapsulate a Vulkan swap chain.
class SwapChain
{
  public:
    friend class Device;
    friend class FramebufferManager;

    SwapChain(const Device& device, uint16_t width, uint16_t height);

    VkFormat get_format() const { return m_format; }

    const VkExtent2D& get_extent() const { return m_extent; }

    ~SwapChain();

  private:
    // Create the image views.
    void create_image_views();

    VkSwapchainKHR m_swap_chain;
    VkFormat m_format;
    VkExtent2D m_extent;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;

    // The device that it's created from.
    const Device& m_device;
};
} // namespace vulkan_scene