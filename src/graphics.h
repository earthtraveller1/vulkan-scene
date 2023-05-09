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

/* Creates a render pass. The boolean value that is returned indicates whether the operation succeeded or not. */
bool create_render_pass(VkRenderPass* p_render_pass);

#endif
