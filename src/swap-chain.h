#ifndef INCLUDED_SWAP_CHAIN_H
#define INCLUDED_SWAP_CHAIN_H

/* A basic abstraction of the Vulkan swap chain. The device class was starting 
to get quite crowded, so I decided to create a new class. */

#include <vulkan/vulkan.h>

struct swap_chain
{
    VkSwapchainKHR swap_chain;
    struct device* device;
};

/* Well, creates a swap chain, obviously. The return type is the failure indic-
ator. It will return true when succeeded, false on failures. */
bool create_new_swap_chain(struct swap_chain* swap_chain, struct device* device, uint16_t width, uint16_t height);

/* Destroys the swap chain. Please note that this needs to be called before the
device that this swap chain is created from gets destroyed. */
void destroy_swap_chain(struct swap_chain* swap_chain);

#endif