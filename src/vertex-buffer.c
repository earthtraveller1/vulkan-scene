

#include "vertex-buffer.h"

extern const VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION = {
    /* binding = */ 0,
    /* stride = */ sizeof(struct vertex),
    /* inputRate = */ VK_VERTEX_INPUT_RATE_VERTEX
};

extern const VkVertexInputAttributeDescription VERTEX_ATTRIBUTE_DESCRIPTIONS[VERTEX_ATTRIBUTE_DESCRIPTION_COUNT] = {
    {
        /* location = */ 0,
        /* binding = */ 0,
        /* format = */ VK_FORMAT_R8G8B8_SRGB,
        /* offset = */ offsetof(struct vertex, position)
    }
};