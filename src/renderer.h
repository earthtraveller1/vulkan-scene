#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include <vulkan/vulkan.h>

/**
 * \file A basic abstraction for rendering with the Vulkan API.
 */

struct rendering_data
{
    const struct device* device;
    const struct swap_chain* swap_chain;
    const struct graphics_pipeline* pipeline;
    const struct framebuffer_manager* framebuffers;
    const struct vertex_buffer* vertex_buffer;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    VkCommandBuffer command_buffer;
};

/* A basic placeholder function. The actual rendering framework will be more
advanced than this later on. */
bool draw(const struct rendering_data* data);

#endif