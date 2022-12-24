#ifndef INCLUDED_SWAP_CHAIN_H
#define INCLUDED_SWAP_CHAIN_H

/* A basic abstraction of the Vulkan swap chain. The device class was starting
to get quite crowded, so I decided to create a new class. */

#include <vulkan/vulkan.h>

#include "device.h"

struct swap_chain
{
    VkSwapchainKHR swap_chain;

    VkFormat format;
    VkExtent2D extent;

    /* Images of the swap chain. */
    VkImage* images;
    uint32_t image_count; /* Vulkan loves uint32_ts */

    /* Image views for the images. */
    VkImageView* image_views;
    /* uint32_t image_view_count; Not needed since the number of image views s-
    hould be the same as the number of images. */

    const struct device* device;
};

/* Well, creates a swap chain, obviously. The return type is the failure indic-
ator. It will return true when succeeded, false on failures. */
bool create_new_swap_chain(struct swap_chain* swap_chain, const struct device* device,
                           uint16_t width, uint16_t height);

/* Destroys the swap chain. Please note that this needs to be called before the
device that this swap chain is created from gets destroyed. */
void destroy_swap_chain(struct swap_chain* swap_chain);

#endif