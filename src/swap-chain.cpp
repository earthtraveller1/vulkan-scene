#include <limits>

#include "device.hpp"

#include "swap-chain.hpp"

using vulkan_scene::SwapChain;
using namespace std::string_literals;

namespace
{
// Obtain the available settings for the swap chain.
std::tuple<VkSurfaceCapabilitiesKHR, std::vector<VkSurfaceFormatKHR>,
           std::vector<VkPresentModeKHR>>
get_swap_chain_support(VkPhysicalDevice p_physical_device,
                       VkSurfaceKHR p_surface)
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_physical_device, p_surface,
                                              &surface_capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                         &format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> surface_formats;
    if (format_count > 0)
    {
        surface_formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                             &format_count,
                                             surface_formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface,
                                              &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes;
    if (present_mode_count > 0)
    {
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface,
                                                  &present_mode_count,
                                                  present_modes.data());
    }

    return {surface_capabilities, surface_formats, present_modes};
}

// Selects the preferred settings for the swap chain.
std::tuple<VkSurfaceFormatKHR, VkPresentModeKHR, VkExtent2D>
choose_swap_chain_configuration(
    const std::tuple<VkSurfaceCapabilitiesKHR, std::vector<VkSurfaceFormatKHR>,
                     std::vector<VkPresentModeKHR>>& p_params,
    uint16_t p_width, uint16_t p_height)
{
    const auto [surface_capabilities, surface_formats, present_modes] =
        p_params;

    VkSurfaceFormatKHR surface_format = surface_formats[0];
    for (const auto& format : surface_formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surface_format = format;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
        }
    }

    VkExtent2D extent =
        (surface_capabilities.currentExtent.width !=
         std::numeric_limits<uint32_t>::max())
            ? surface_capabilities.currentExtent
            : VkExtent2D{.width = std::clamp(
                             static_cast<uint32_t>(p_width),
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width),
                         .height = std::clamp(
                             static_cast<uint32_t>(p_height),
                             surface_capabilities.minImageExtent.height,
                             surface_capabilities.maxImageExtent.height)};

    return {surface_format, present_mode, extent};
}
} // namespace

SwapChain::SwapChain(VkPhysicalDevice p_physical_device, const Device& p_device,
                     VkSurfaceKHR p_surface, uint16_t p_width,
                     uint16_t p_height, uint32_t p_graphics_family,
                     uint32_t p_present_family): m_device(p_device)
{
    const auto support = get_swap_chain_support(p_physical_device, p_surface);
    const auto [surface_capabilities, surface_formats, present_modes] = support;
    const auto [surface_format, present_mode, extent] =
        choose_swap_chain_configuration(support, p_width, p_height);

    m_format = surface_format.format;
    m_extent = extent;

    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0)
    {
        if (image_count > surface_capabilities.maxImageCount)
        {
            image_count = surface_capabilities.maxImageCount;
        }
    }

    VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE};

    const uint32_t queue_families[2] = {p_graphics_family, p_present_family};

    if (p_graphics_family == p_present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_families;
    }

    const auto result = vkCreateSwapchainKHR(
        p_device.get_raw_handle(), &create_info, nullptr, &m_swap_chain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create a Vulkan swap chain. Vulkan error "s +
            std::to_string(result) + '.');
    }
}

SwapChain::~SwapChain() {
    vkDestroySwapchainKHR(m_device.get_raw_handle(), m_swap_chain, nullptr);
}