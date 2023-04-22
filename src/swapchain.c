#include <stdlib.h>

#include <vulkan/vulkan_core.h>

#include "device.h"

#include "swapchain.h"

struct swap_chain_support_info get_swap_chain_support_info(VkPhysicalDevice physical_device)
{
    struct swap_chain_support_info support_info;

    VkSurfaceKHR surface = get_global_surface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support_info.surface_capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &support_info.surface_format_count, NULL);
    if (support_info.surface_format_count)
    {
        support_info.surface_formats = malloc(support_info.surface_format_count * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &support_info.surface_format_count, support_info.surface_formats);
    }
    else
    {
        support_info.surface_formats = NULL;
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &support_info.present_mode_count, NULL);
    if (support_info.present_mode_count)
    {
        support_info.present_modes = malloc(support_info.surface_format_count * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &support_info.present_mode_count, support_info.present_modes);
    }
    else
    {
        support_info.present_modes = NULL;
    }

    return support_info;
}

void destroy_swap_chain_support_info(const struct swap_chain_support_info* support_info)
{
    free(support_info->surface_formats);
    free(support_info->present_modes);
}
