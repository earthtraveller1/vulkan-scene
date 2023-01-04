#ifndef INCLUDED_GRAPHICS_PIPELINE_H
#define INCLUDED_GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.h>

#include "device.h"
#include "math.h"
#include "swap-chain.h"


/* Represents an abstraction over the graphics pipeline. */

struct graphics_pipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkRenderPass render_pass;

    const struct device* device;
};

/* In the future, I would like the user to specify their own push constants, b-
ut for now, no. */
struct pipeline_push_constants_v
{
    struct matrix_4 projection;
};

struct pipeline_push_constants_f
{
    float color_shift_amount;
};

/* Creates a new pipeline. */
bool create_new_graphics_pipeline(struct graphics_pipeline* pipeline,
                                  const struct device* device,
                                  const struct swap_chain* swap_chain,
                                  const char* vertex_shader_path,
                                  const char* fragment_shader_path);

/**
 * \brief Binds the graphics pipeline to the command buffer. The command buffer
 * must have begun, or else it's undefined behaviour!
 *
 * \param pipeline The graphics pipeline to bind.
 * \param cmd_buffer The command buffer to bind to.
 */
void bind_graphics_pipeline(struct graphics_pipeline* pipeline,
                            VkCommandBuffer cmd_buffer);

/**
 * \brief Sets the push constants for the fragment shader.
 *
 * \param self The graphics pipeline to set the push constants.
 * \param cmd_buffer The command buffer that is in use right now.
 * \param constants The values of the push constants.
 */
void set_graphics_pipeline_push_constants_f(
    const struct graphics_pipeline* self, VkCommandBuffer cmd_buffer,
    const struct pipeline_push_constants_f* constants);

/**
 * \brief Sets the push constants for the vertex shader.
 *
 * \param self The graphics pipeline to set the push constants.
 * \param cmd_buffer The command buffer that is in use right now.
 * \param constants The values of the push constants.
 */
void set_graphics_pipeline_push_constants_v(
    const struct graphics_pipeline* self, VkCommandBuffer cmd_buffer,
    const struct pipeline_push_constants_v* constants);

/* Destroys the graphics pipeline. */
void destroy_graphics_pipeline(struct graphics_pipeline* pipeline);

#endif