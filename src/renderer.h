#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include <vulkan/vulkan.h>

#include "buffers.h"
#include "device.h"
#include "framebuffer-manager.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"

/**
 * \file A basic abstraction for rendering with the Vulkan API.
 */

struct renderer
{
    struct device device;
    struct swap_chain swap_chain;

    struct window* window;

    /* In the future, we will support having multiple pipelines, but for now,
    let's keep things simple. */
    struct graphics_pipeline pipeline;

    /* We only allow for one call to load_vertex_data, but that's gonna change
    in the future. */
    bool vertex_buffer_valid;
    struct buffer vertex_buffer;

    /* Same thing with load_indices. */
    bool index_buffer_valid;
    struct buffer index_buffer;

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
 * \brief Loads indices into the renderer.
 *
 * \param self The renderer to load the indices into.
 * \param index_count The number of indices to be loaded.
 * \param indices A pointer to the array of indices to be loaded in.
 *
 * \returns A boolean that indicates failure with `false`.
 */
bool load_indices_into_renderer(struct renderer* self, size_t index_count,
                                const uint32_t* indices);

/**
 * \brief Start rendering.
 *
 * \param self The renderer to use for rendering.
 * \param recreate_swap_chain The indication of whether to recreate the swap
 * chain or not.
 *
 * \returns `true` if no errors occured, `false` if otherwise.
 */
bool begin_renderer(struct renderer* self, bool* recreate_swap_chain);

bool recreate_renderer_swap_chain(struct renderer* self);

/* Draws a triangle with the specified renderer. */
void draw_triangle(struct renderer* self);

/* Draws a polygon with the vertex and indices loaded. */
void draw_polygon(struct renderer* self, uint32_t vertex_count,
                  const struct pipeline_push_constants_v* push_constants_v,
                  const struct pipeline_push_constants_f* push_constants_f);

/* Stops rendering and submit everything. */
bool end_renderer(struct renderer* self, bool* recreate_swap_chain);

/**
 * \brief The destructor for the renderer object. Must be called to prevent
 * memory leaks.
 *
 * \param self The renderer that should be destroyed.
 */
void destroy_renderer(struct renderer* self);

#endif