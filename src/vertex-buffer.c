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

static uint32_t get_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device, bool* found)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            *found = true;
            return i;
        }
    }
    
    *found = false;
}

bool create_vertex_buffer(struct vertex_buffer* self, struct device* device, const struct vertex* data, size_t data_len)
{
    self->device = device;
    
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
    
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->device, self->buffer, &memory_requirements);
    
    bool found_memory_type;
    uint32_t memory_type = get_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device->physical_device, &found_memory_type);
    if (!found_memory_type)
    {
        fputs("[ERROR]: Was unable to locate an adequate memory type for the buffer.\n", stderr);
        return false;
    }
    
    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = data_len * sizeof(struct vertex);
    allocate_info.memoryTypeIndex = memory_type;
    
    result = vkAllocateMemory(device->device, &allocate_info, NULL, &self->memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to allocate memory. Vulkan error %d.\n", result);
        return false;
    }
}

void destroy_vertex_buffer(struct vertex_buffer* self)
{
    vkFreeMemory(self->device->device, self->memory, NULL);
    vkDestroyBuffer(self->device->device, self->buffer, NULL);
}