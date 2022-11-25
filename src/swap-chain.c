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

/* The width and height will be used whenever the surface doesn't contain the  
swap chain extent information. */
static void choose_swap_chain_settings(struct support_details* support_details, uint16_t width, uint16_t height, VkSurfaceFormatKHR* surface_format, VkPresentModeKHR* present_mode, VkExtent2D* extent)
{
    /* Surface format. */
    
    bool found_surface_format = false;
    
    for (const VkSurfaceFormatKHR* sf = support_details->surface_formats; sf < support_details->surface_formats + support_details->surface_format_count; sf++)
    {
        if (sf->format == VK_FORMAT_R8G8B8A8_SRGB && sf->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            *surface_format = *sf;
            found_surface_format = true;
            break;
        }
    }
    
    if (!found_surface_format)
    {
        *surface_format = support_details->surface_formats[0];
    }
    
    /* The present mode. */
    
    *present_mode = VK_PRESENT_MODE_FIFO_KHR;
    
    for (const VkPresentModeKHR* pm = support_details->present_modes; pm < support_details->present_modes + support_details->present_mode_count; pm++)
    {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            *present_mode = *pm;
        }
    }
    
    /* The swap chain extent. */
    
    if (support_details->surface_capabilties.currentExtent.width < UINT32_MAX)
    {
        *extent = support_details->surface_capabilties.currentExtent;
    }
    else 
    {
        extent->width = width;
        extent->height = height;
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