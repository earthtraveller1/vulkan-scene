#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include <vulkan/vulkan.h>

#include "device.h"
#include "framebuffer-manager.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"
#include "buffers.h"

/**
 * \file A basic abstraction for rendering with the Vulkan API.
 */

struct renderer
{
    struct device device;
    struct swap_chain swap_chain;

    /* In the future, we will support having multiple pipelines, but for now,
    let's keep things simple. */
    struct graphics_pipeline pipeline;
    
    /* We only allow for one call to load_vertex_data, but that's gonna change 
    in the future. */
    bool vertex_buffer_valid;
    struct vertex_buffer vertex_buffer;

    struct framebuffer_manager framebuffers;

    /* Synchronization objects. They are simple enough that we don't need to c-
    reate wrapper structs around them. */
    struct
    {
        VkSemaphore image_available;
        VkSemaphore render_finished;
    } semaphores;
    VkFence frame_fence;
    
    VkCommandBuffer command_buffer;
    
    uint32_t image_index;
};

/**
 * \brief Creates a new renderer object.
 *
 * \param self The renderer that would be created.
 * \param window The window that the renderer will be targeting.
 * \param app_name The name of the application.
 * \param enable_validation Whether to enable validation or not.
 * \param vertex_shader_path The path to the compiled SPIR-V of the vertex
 * shader. \param fragment_shader_path The path to the compiled SPIR-V of the
 * fragment shader.
 */
bool create_new_renderer(struct renderer* self, struct window* window,
                         const char* app_name, bool enable_validation,
                         const char* vertex_shader_path,
                         const char* fragment_shader_path);

/**
 * \brief Loads vertex data into the renderer.
 *
 * \param self The renderer that the vertex data would be loaded into.
 * \param vertex_count The number of vertices to be loaded in.
 * \param vertices A pointer to an array of vertices to be loaded in.
 */
bool load_vertex_data_into_renderer(struct renderer* self, size_t vertex_count,
                                    const struct vertex* vertices);

/**
 * \brief Start rendering.
 * 
 * \param self The renderer to use for rendering.
 * 
 * \returns `true` if no errors occured, `false` if otherwise.
*/
bool begin_renderer(struct renderer* self);

/* Draws a triangle with the specified renderer. */
void draw_triangle(struct renderer* self);

/* Stops rendering and submit everything. */
bool end_renderer(struct renderer* self);

/**
 * \brief The destructor for the renderer object. Must be called to prevent
 * memory leaks.
 *
 * \param self The renderer that should be destroyed.
 */
void destroy_renderer(struct renderer* self);

#endif