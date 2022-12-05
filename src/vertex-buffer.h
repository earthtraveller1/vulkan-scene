#ifndef INCLUDED_VERTEX_BUFFER_H
#define INCLUDED_VERTEX_BUFFER_H

#include <vulkan/vulkan.h>

#include "math.h"

/* An abstraction for working with Vulkan vertex buffers, and some utility fun-
ctions for stuff like vertex input. */

struct vertex
{
    struct vector_3 position;
};

#define VERTEX_ATTRIBUTE_DESCRIPTION_COUNT 1

extern const VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION;

extern const VkVertexInputAttributeDescription VERTEX_ATTRIBUTE_DESCRIPTIONS[VERTEX_ATTRIBUTE_DESCRIPTION_COUNT];

#endif