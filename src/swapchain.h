/* Since there's probably going to be only a single swapchain for the entire time of the program,
 * I will make it a static variable within the swapchain.c file. */

#ifndef INCLUDED_swapchain_h
#define INCLUDED_swapchain_h

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

/* A struct containing information regarding swapchain support. And, yes, a lot of this is in fact based on vulkan-tutorial.com. Hey, look,
 * I can't come up with a better approach to this. */
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
bool create_swap_chain(void);

/* Creates an array of framebuffers, one for each image in the swap chain. */
bool create_swap_chain_framebuffers(VkRenderPass render_pass, VkFramebuffer** framebuffers, uint32_t* framebuffer_count);

/* Retrive an image from the swap chain to draw on. */
bool swap_chain_acquire_next_image(uint32_t* p_image_index, VkSemaphore p_semaphore);

/* Returns the swap chain format in the form of a VkFormat. */
VkFormat get_swap_chain_format(void);

/* Destroys an array of framebuffers and frees the memory as well. */
void destroy_swap_chain_framebuffers(VkFramebuffer* framebuffers, uint32_t framebuffer_count);

/* Destroys the swap chain */
void destroy_swapchain(void);

#endif
