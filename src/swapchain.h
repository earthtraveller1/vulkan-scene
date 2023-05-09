/* Since there's probably going to be only a single swapchain for the entire time of the program,
 * I will make it a static variable within the swapchain.c file. */

#ifndef INCLUDED_swapchain_h
#define INCLUDED_swapchain_h

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

/* A struct containing information regarding swapchain support. And, yes, a lot of this is in fact based on vulkan-tutorial.com. Hey, look, I can't come up with
 * a better approach to this. */
struct swap_chain_support_info
{
    VkSurfaceCapabilitiesKHR surface_capabilities;

    VkSurfaceFormatKHR* surface_formats;
    uint32_t surface_format_count;

    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;
};

/* Obtains information regarding swap chain support of a specific physical device. */
struct swap_chain_support_info get_swap_chain_support_info(VkPhysicalDevice physical_device);

/* Destructor for the swap_chain_support_info struct. The struct is no longer valid after this call and MUST NOT BE USED. */
void destroy_swap_chain_support_info(const struct swap_chain_support_info* support_info);

/* Creates the swap chain */
bool create_swapchain();

/* Returns the swap chain format in the form of a VkFormat. */
VkFormat get_swap_chain_format();

/* Destroys the swap chain */
void destroy_swapchain();

#endif
