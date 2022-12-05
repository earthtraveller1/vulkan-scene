#include <stdbool.h>
#include <stdio.h>

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
        /* format = */ VK_FORMAT_R32G32B32_SFLOAT,
        /* offset = */ offsetof(struct vertex, position)
    }
};

bool create_vertex_buffer(struct vertex_buffer* self, struct device* device, const struct vertex* data, size_t data_len)
{
    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.size = data_len * sizeof(struct vertex);
    create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = NULL;
    
    VkResult result = vkCreateBuffer(device->device, &create_info, NULL, &self->buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a vertex buffer. Vulkan error %d.\n", result);
        return false;
    }
}