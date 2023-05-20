#ifndef INCLUDED_graphics_h
#define INCLUDED_graphics_h

#include <stdbool.h>

#include <vulkan/vulkan.h>

/* This file contains all of the general purposed graphics-related components, including the graphics pipeline, the buffers,
 * the textures, etc. */

/* A single vertex. */
struct vertex
{
    float position[3];
};

struct graphics_pipeline
{
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

enum buffer_type
{
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_STAGING
};

struct buffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;
};

/* Creates a render pass. The boolean value that is returned indicates whether the operation succeeded or not. */
bool create_render_pass(VkRenderPass* p_render_pass);

/* Creates a graphics pipeline. You do in fact need a render pass created before this, of course. */
bool create_graphics_pipeline(const char* p_vertex_path, const char* p_fragment_path, VkRenderPass p_render_pass,
                              struct graphics_pipeline* p_pipeline);

bool create_buffer(const void* p_buffer_data, size_t p_buffer_size, enum buffer_type p_buffer_type, struct buffer* p_buffer);

void destroy_graphics_pipeline(const struct graphics_pipeline* p_pipeline);

void destroy_render_pass(VkRenderPass p_render_pass);

void destroy_buffer(const struct buffer* p_buffer);

#endif
