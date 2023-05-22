#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan_core.h>

#include "device.h"
#include "window.h"

#include "swapchain.h"

static VkSwapchainKHR swap_chain;

/* The images inside of the swap chain */
static uint32_t swap_chain_image_count;
static VkImage* swap_chain_images; /* Vulkan loves uint32_ts for some reasons */

/* The images views. We don't need a count for these since its length should be the
 * same as the image array. */
static VkImageView* swap_chain_image_views;

/* Information about the swap chain that we need for the future */
static VkFormat swap_chain_format;
static VkExtent2D swap_chain_extent;

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

static void choose_swap_chain_settings(const struct swap_chain_support_info* swap_chain_info, VkSurfaceFormatKHR* surface_format,
                                       VkPresentModeKHR* present_mode, VkExtent2D* swap_extent)
{
    *surface_format = swap_chain_info->surface_formats[0];

    for (const VkSurfaceFormatKHR* format = swap_chain_info->surface_formats;
         format < swap_chain_info->surface_formats + swap_chain_info->surface_format_count; format++)
    {
        if (format->format == VK_FORMAT_B8G8R8A8_SRGB && format->colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            *surface_format = *format;
        }
    }

    *present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const VkPresentModeKHR* mode = swap_chain_info->present_modes;
         mode < swap_chain_info->present_modes + swap_chain_info->present_mode_count; mode++)
    {
        if (*mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            *present_mode = *mode;
        }
    }

    if (swap_chain_info->surface_capabilities.currentExtent.width != UINT32_MAX)
    {
        *swap_extent = swap_chain_info->surface_capabilities.currentExtent;
    }
    else
    {
        int width, height;
        get_framebuffer_size(&width, &height);

        swap_extent->width = clamp((uint32_t)width, swap_chain_info->surface_capabilities.minImageExtent.width,
                                   swap_chain_info->surface_capabilities.maxImageExtent.width);
        swap_extent->height = clamp((uint32_t)height, swap_chain_info->surface_capabilities.minImageExtent.height,
                                    swap_chain_info->surface_capabilities.maxImageExtent.height);
    }
}

void destroy_swap_chain_support_info(const struct swap_chain_support_info* support_info)
{
    if (support_info->surface_format_count)
        free(support_info->surface_formats);
    if (support_info->present_mode_count)
        free(support_info->present_modes);
}

static bool create_image_views(void)
{
    swap_chain_image_views = malloc(swap_chain_image_count * sizeof(VkImageView));

    VkDevice device = get_global_logical_device();

    for (uint32_t i = 0; i < swap_chain_image_count; i++)
    {
        VkImageViewCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = 0;
        create_info.image = swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swap_chain_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device, &create_info, NULL, swap_chain_image_views + i);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "\033[91m[ERROR]: Failed to create an image view. Vulkan error %d\033[0m\n", result);
            return false;
        }
    }

    return true;
}

bool create_swap_chain(void)
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
        /* 0 indicates that there is no maximum. */
        if (support_info.surface_capabilities.maxImageCount != 0)
        {
            create_info.minImageCount = support_info.surface_capabilities.maxImageCount;
        }
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

    VkDevice device = get_global_logical_device();

    VkResult result = vkCreateSwapchainKHR(device, &create_info, NULL, &swap_chain);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create the swap chain. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    /* Retrieve the images from the swap chain. */
    vkGetSwapchainImagesKHR(device, swap_chain, &swap_chain_image_count, NULL);
    swap_chain_images = malloc(swap_chain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swap_chain, &swap_chain_image_count, swap_chain_images);

    /* Retrieve the format and the extent */
    swap_chain_format = surface_format.format;
    swap_chain_extent = swap_extent;

    /* Create the image views */
    create_image_views();

    /* Cleanup */
    destroy_swap_chain_support_info(&support_info);

    return true;
}

bool create_swap_chain_framebuffers(VkRenderPass p_render_pass, VkFramebuffer** p_framebuffers, uint32_t* p_framebuffer_count)
{
    *p_framebuffer_count = swap_chain_image_count;
    *p_framebuffers = malloc(*p_framebuffer_count * sizeof(VkFramebuffer));

    VkDevice device = get_global_logical_device();

    for (uint32_t i = 0; i < *p_framebuffer_count; i++)
    {
        VkFramebufferCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = 0;
        create_info.renderPass = p_render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &swap_chain_image_views[i];
        create_info.width = swap_chain_extent.width;
        create_info.height = swap_chain_extent.height;
        create_info.layers = 1;

        VkResult result = vkCreateFramebuffer(device, &create_info, NULL, *p_framebuffers + i);

        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "\033[91m[ERROR]: Failed to create swap chain framebuffers. Vulkan error %d.\033[0m\n", result);
            return false;
        }
    }

    return true;
}

VkSwapchainKHR get_global_swap_chain(void) { return swap_chain; }

VkFormat get_swap_chain_format(void) { return swap_chain_format; }

void destroy_swap_chain_framebuffers(VkFramebuffer* p_framebuffers, uint32_t p_framebuffer_count)
{
    VkDevice device = get_global_logical_device();

    for (const VkFramebuffer* framebuffer = p_framebuffers; framebuffer < p_framebuffers + p_framebuffer_count; framebuffer++)
    {
        vkDestroyFramebuffer(device, *framebuffer, NULL);
    }

    free(p_framebuffers);
}

void destroy_swapchain(void)
{
    for (const VkImageView* image_view = swap_chain_image_views; image_view < swap_chain_image_views + swap_chain_image_count; image_view++)
    {
        vkDestroyImageView(get_global_logical_device(), *image_view, NULL);
    }

    vkDestroySwapchainKHR(get_global_logical_device(), swap_chain, NULL);

    free(swap_chain_image_views);
    free(swap_chain_images);
}
