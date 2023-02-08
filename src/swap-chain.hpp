#pragma once

#include "device.hpp"
#include "utils.hpp"

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

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    VkFormat get_format() const { return m_format; }

    const VkExtent2D& get_extent() const { return m_extent; }
    
    // Acquires an image from the swap chain. This function is specifically suited
    // for the Renderer class.
    inline uint32_t acquire_next_image(VkSemaphore semaphore) const
    {
        uint32_t image_index;
        vkAcquireNextImageKHR(m_device.get_raw_handle(), m_swap_chain,
                              (std::numeric_limits<uint64_t>::max)(), semaphore,
                              VK_NULL_HANDLE, &image_index);
        return image_index;
    }
    
    // Presents an image to the screen.
    inline void present(VkSemaphore wait_semaphore, uint32_t image_index) const
    {
        const VkPresentInfoKHR present_info {
          .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
          .pNext = nullptr,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &wait_semaphore,
          .swapchainCount = 1,
          .pSwapchains = &m_swap_chain,
          .pImageIndices = &image_index,
          .pResults = nullptr
        };
        
        const auto result = vkQueuePresentKHR(m_device.get_present_queue(), &present_info);
        vulkan_scene_VK_CHECK(result, "present a swap chain");
    }

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