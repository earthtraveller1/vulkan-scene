#ifndef INCLUDED_VERTEX_BUFFER_H
#define INCLUDED_VERTEX_BUFFER_H

#include <vulkan/vulkan.h>

#include "math.h"

/* An abstraction for working with Vulkan vertex buffers, and some utility fun-
ctions for stuff like vertex input. */

struct vertex
{
    vector_3 position;
};

extern const VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION;

#endif