#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#include "swap-chain.h"

struct support_details
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    uint32_t surface_format_count; /* Vulkan seems to really like uint32_t, as
                                      they use it for everything. */
    VkSurfaceFormatKHR* surface_formats;
    uint32_t present_mode_count;
    VkPresentModeKHR* present_modes;
};

static void get_support_details(struct support_details* support_details,
                                VkPhysicalDevice physical_device,
                                VkSurfaceKHR surface)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &(support_details->surface_capabilities));

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &(support_details->surface_format_count),
        NULL);
    if (support_details->surface_format_count > 0)
    {
        support_details->surface_formats =
            malloc(support_details->surface_format_count *
                   sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, &(support_details->surface_format_count),
            support_details->surface_formats);
    }
    else
    {
        support_details->surface_formats = NULL; /* For safety. */
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &(support_details->present_mode_count), NULL);
    if (support_details->present_mode_count > 0)
    {
        support_details->present_modes = malloc(
            support_details->present_mode_count * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, &(support_details->present_mode_count),
            support_details->present_modes);
    }
    else
    {
        support_details->present_modes = NULL; /* For safety. */
    }
}

/* The width and height will be used whenever the surface doesn't contain the
swap chain extent information. */
static void choose_swap_chain_settings(struct support_details* support_details,
                                       uint16_t width, uint16_t height,
                                       VkSurfaceFormatKHR* surface_format,
                                       VkPresentModeKHR* present_mode,
                                       VkExtent2D* extent)
{
    /* Surface format. */

    bool found_surface_format = false;

    for (const VkSurfaceFormatKHR* sf = support_details->surface_formats;
         sf < support_details->surface_formats +
                  support_details->surface_format_count;
         sf++)
    {
        if (sf->format == VK_FORMAT_R8G8B8A8_SRGB &&
            sf->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

    for (const VkPresentModeKHR* pm = support_details->present_modes;
         pm <
         support_details->present_modes + support_details->present_mode_count;
         pm++)
    {
        if (*pm == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            *present_mode = *pm;
        }
    }

    /* The swap chain extent. */

    if (support_details->surface_capabilities.currentExtent.width < UINT32_MAX)
    {
        *extent = support_details->surface_capabilities.currentExtent;
    }
    else
    {
        extent->width = width;
        extent->height = height;
    }
}

static bool create_image_views(struct swap_chain* swap_chain)
{
    swap_chain->image_views =
        malloc(sizeof(VkImageView) * swap_chain->image_count);

    for (uint32_t i = 0; i < swap_chain->image_count; i++)
    {
        VkImageViewCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = 0;
        create_info.image = swap_chain->images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swap_chain->format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult result =
            vkCreateImageView(swap_chain->device->device, &create_info, NULL,
                              swap_chain->image_views + i);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr,
                    "[FATAL ERROR]: Failed to create image view %d for a swap "
                    "chain. Vulkan error %d.\n",
                    i, result);
            return false;
        }
    }

    return true;
}

bool create_new_swap_chain(struct swap_chain* swap_chain, struct device* device,
                           uint16_t width, uint16_t height)
{
    struct support_details support_details;
    get_support_details(&support_details, device->physical_device,
                        device->surface);

    VkPresentModeKHR present_mode;
    VkSurfaceFormatKHR surface_format;
    VkExtent2D swap_chain_extent;

    choose_swap_chain_settings(&support_details, width, height, &surface_format,
                               &present_mode, &swap_chain_extent);

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.surface = device->surface;

    if (support_details.surface_capabilities.maxImageCount != 0 &&
        support_details.surface_capabilities.minImageCount + 1 >
            support_details.surface_capabilities.maxImageCount)
    {
        create_info.minImageCount =
            support_details.surface_capabilities.minImageCount + 1;
    }
    else
    {
        create_info.minImageCount =
            support_details.surface_capabilities.maxImageCount;
    }

    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = swap_chain_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queue_families[2] = {device->graphics_queue_family,
                                        device->present_queue_family};

    if (device->present_queue_family == device->graphics_queue_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_families;
    }

    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device->device, &create_info, NULL,
                                           &swap_chain->swap_chain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[FATAL ERROR]: Failed to create a Vulkan swapchain. Vulkan "
                "error %d.\n",
                result);
        return false;
    }

    swap_chain->device = device;
    swap_chain->format = surface_format.format;
    swap_chain->extent = swap_chain_extent;

    vkGetSwapchainImagesKHR(device->device, swap_chain->swap_chain,
                            &swap_chain->image_count, NULL);
    swap_chain->images = malloc(sizeof(VkImage) * swap_chain->image_count);
    vkGetSwapchainImagesKHR(device->device, swap_chain->swap_chain,
                            &swap_chain->image_count, swap_chain->images);

    create_image_views(swap_chain);

    /* TODO: These must be at the very end of the function, for safety. Optimi-
    sation will come later. */
    free(support_details.surface_formats);
    free(support_details.present_modes);

    return true;
}

void destroy_swap_chain(struct swap_chain* swap_chain)
{
    for (const VkImageView* image_view = swap_chain->image_views;
         image_view < swap_chain->image_views + swap_chain->image_count;
         image_view++)
    {
        vkDestroyImageView(swap_chain->device->device, *image_view, NULL);
    }

    vkDestroySwapchainKHR(swap_chain->device->device, swap_chain->swap_chain,
                          NULL);

    free(swap_chain->images);
    free(swap_chain->image_views);
}