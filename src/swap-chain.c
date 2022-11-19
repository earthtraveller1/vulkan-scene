#include <stddef.h>
#include <stdint.h>

#include "device.h"

#include "swap-chain.h"

static struct support_details
{
    VkSurfaceCapabilitiesKHR surface_capabilties;
    uint32_t surface_format_count; /* Vulkan seems to really like uint32_t, as they use it for everything. */
    VkSurfaceFormatKHR* surface_formats;
    uint32_t present_mode_count;
    VkPresentModeKHR* present_modes;
};

static void get_support_details(struct support_details* support_details, VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &(support_details->surface_capabilties));
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &(support_details->surface_format_count), NULL);
    if (support_details->surface_format_count > 0)
    {
        support_details->surface_formats = malloc(support_details->surface_format_count * sizeof(VkSurfaceCapabilitiesKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &(support_details->surface_format_count), support_details->surface_formats);
    }
    else 
    {
        support_details->surface_formats = NULL; /* For safety. */
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &(support_details->present_mode_count), NULL);
    if (support_details->present_mode_count > 0)
    {
        support_details->present_modes = malloc(support_details->present_mode_count * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &(support_details->present_mode_count), support_details->present_modes);
    }
    else
    {
        support_details->present_modes = NULL; /* For safety. */
    }
}

void create_new_swap_chain(struct swap_chain* swap_chain, struct device* device)
{
    struct support_details support_details;
    get_support_details(&support_details, device->physical_device, device->surface);
    
    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.surface = device->surface;
    
    /* TODO: These must be at the very end of the function, for safety. Optimi-
    sation will come later. */
    free(support_details.surface_formats);
    free(support_details.present_modes);
}