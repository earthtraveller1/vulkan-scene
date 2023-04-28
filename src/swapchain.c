#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan_core.h>

#include "device.h"
#include "window.h"

#include "swapchain.h"

static VkSwapchainKHR swap_chain;

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

/* A basic macro for clamping a value within a certain range. Currently only used within this file, though
 * if other files needs it, I might move it to a public scope. */
static uint32_t clamp(uint32_t value, uint32_t min, uint32_t max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

static void choose_swap_chain_settings(const struct swap_chain_support_info* swapchain_info, VkSurfaceFormatKHR* surface_format,
                                       VkPresentModeKHR* present_mode, VkExtent2D* swap_extent)
{
    *surface_format = swapchain_info->surface_formats[0];

    for (const VkSurfaceFormatKHR* format = swapchain_info->surface_formats;
         format < swapchain_info->surface_formats + swapchain_info->surface_format_count; format++)
    {
        if (format->format == VK_FORMAT_B8G8R8A8_SRGB && format->colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            *surface_format = *format;
        }
    }

    *present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const VkPresentModeKHR* mode = swapchain_info->present_modes;
         mode < swapchain_info->present_modes + swapchain_info->present_mode_count; mode++)
    {
        if (*mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            *present_mode = *mode;
        }
    }

    if (swapchain_info->surface_capabilities.currentExtent.width != UINT32_MAX)
    {
        *swap_extent = swapchain_info->surface_capabilities.currentExtent;
    }
    else
    {
        int width, height;
        get_framebuffer_size(&width, &height);

        swap_extent->width = clamp((uint32_t)width, swapchain_info->surface_capabilities.minImageExtent.width,
                                   swapchain_info->surface_capabilities.maxImageExtent.width);
        swap_extent->height = clamp((uint32_t)height, swapchain_info->surface_capabilities.minImageExtent.height,
                                    swapchain_info->surface_capabilities.maxImageExtent.height);
    }
}

void destroy_swap_chain_support_info(const struct swap_chain_support_info* support_info)
{
    free(support_info->surface_formats);
    free(support_info->present_modes);
}

bool create_swapchain()
{
    struct swap_chain_support_info support_info = get_swap_chain_support_info(get_global_physical_device());

    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D swap_extent;

    choose_swap_chain_settings(&support_info, &surface_format, &present_mode, &swap_extent);

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.surface = get_global_surface();

    create_info.minImageCount = support_info.surface_capabilities.minImageCount + 1;

    /* Make sure that we don't create more than the maximum. */
    if (create_info.minImageCount > support_info.surface_capabilities.maxImageCount)
    {
        create_info.minImageCount = support_info.surface_capabilities.maxImageCount;
    }

    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swap_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /* The first element is the graphics queue family, and the second element is the present one */
    uint32_t queue_families[2];
    get_global_queue_families(queue_families, queue_families + 1);

    /* Check if the queue families are unique, meaning, are they different. This is
     * necessary because both the graphics queue and the present queue needs to access
     * the images within the swap chain. */
    if (queue_families[0] == queue_families[1])
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_families;
    }

    create_info.preTransform = support_info.surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; /* We don't want the surface to be transparent. */
    create_info.presentMode = present_mode;
    create_info.clipped = VK_FALSE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(get_global_logical_device(), &create_info, NULL, &swap_chain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create the swap chain. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

void destroy_swapchain()
{
    vkDestroySwapchainKHR(get_global_logical_device(), swap_chain, NULL);
}
