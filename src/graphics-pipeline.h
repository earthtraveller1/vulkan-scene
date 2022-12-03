#ifndef INCLUDED_GRAPHICS_PIPELINE_H
#define INCLUDED_GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.h>

#include "device.h"

/* Represents an abstraction over the graphics pipeline. */

struct graphics_pipeline
{
    VkPipeline pipeline;
};

/* Creates a new pipeline. */
void create_new_graphics_pipeline(struct graphics_pipeline* pipeline, struct device* device);

/* Destroys the graphics pipeline. */
void destroy_graphics_pipeline(struct graphics_pipeline* pipeline);

#endif