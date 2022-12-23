#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include "device.h"
#include "swap-chain.h"

/**
 * \file A basic abstraction for rendering with the Vulkan API.
 */

/* struct rendering_data
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
advanced than this later on. /
bool draw(const struct rendering_data* data); */

struct renderer
{
    struct device device;
    struct swap_chain swap_chain;
};

/**
 * \brief Creates a new renderer object.
 * 
 * \param self The renderer that would be created.
 * \param window The window that the renderer will be targeting.
 * \param app_name The name of the application.
 * \param enable_validation Whether to enable validation or not.
 * \param vertex_shader_path The path to the compiled SPIR-V of the vertex shader.
 * \param fragment_shader_path The path to the compiled SPIR-V of the fragment shader.
*/
bool create_new_renderer(struct renderer* self, struct window* window, const char* app_name, bool enable_validation, const char* vertex_shader_path, const char* fragment_shader_path);

/**
 * \brief The destructor for the renderer object. Must be called to prevent memory leaks.
 * 
 * \param self The renderer that should be destroyed.
*/
void destroy_renderer(struct renderer* self);

#endif