#pragma once

namespace vulkan_scene
{
class Device;

// A basic class that can encapsulate a Vulkan swap chain.
class SwapChain
{
  public:
    friend class Device;

    ~SwapChain();

  private:
    // The constructor is private to make swap chain creation safer.
    SwapChain(VkPhysicalDevice physical_device, const Device& device,
              VkSurfaceKHR surface, uint16_t width, uint16_t height,
              uint32_t graphics_family, uint32_t present_family);

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