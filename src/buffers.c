#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "device.h"

#include "graphics.h"

enum buffer_type
{
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_STAGING
};

static bool find_memory_type(uint32_t p_type_filter, VkMemoryPropertyFlags p_property_flags, uint32_t* p_type_index)
{
    VkPhysicalDevice physical_device = get_global_physical_device();
    VkPhysicalDeviceMemoryProperties device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &device_memory_properties);

    for (uint32_t i = 0; i < device_memory_properties.memoryTypeCount; i++)
    {
        if (p_type_filter & (1 << i))
        {
            if ((device_memory_properties.memoryTypes[i].propertyFlags & p_property_flags) == p_property_flags)
            {
                *p_type_index = i;
                return true;
            }
        }
    }

    fprintf(stderr, "\033[91m[ERROR]: Failed to find a suitable memory type for a buffer.\033[0m\n");
    return false;
}

static bool create_buffer(size_t p_size, enum buffer_type p_type, VkBuffer* p_buffer, VkDeviceMemory* p_memory)
{
    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.size = p_size;

    VkMemoryPropertyFlags memory_property_flags;

    switch (p_type)
    {
    case BUFFER_TYPE_VERTEX:
        create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case BUFFER_TYPE_INDEX:
        create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case BUFFER_TYPE_STAGING:
        create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memory_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    }

    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = NULL;

    VkDevice device = get_global_logical_device();

    VkResult result = vkCreateBuffer(device, &create_info, NULL, p_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create a buffer. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, *p_buffer, &memory_requirements);

    uint32_t memory_type_index;
    if (!find_memory_type(memory_requirements.memoryTypeBits, memory_property_flags, &memory_type_index))
        return false;

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;

    result = vkAllocateMemory(device, &allocate_info, NULL, p_memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to allocate memory for a buffer. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    vkBindBufferMemory(device, *p_buffer, *p_memory, 0);

    return true;
}

bool create_vertex_buffer(const struct vertex* p_vertices, size_t p_vertex_count, struct buffer* p_buffer)
{
    VkDevice device = get_global_logical_device();
    const size_t buffer_size = p_vertex_count * sizeof(struct vertex);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    if (!create_buffer(p_vertex_count * sizeof(struct vertex), BUFFER_TYPE_STAGING, &staging_buffer, &staging_buffer_memory))
        return false;

    void* staging_buffer_ptr;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &staging_buffer_ptr);
    memcpy(staging_buffer_ptr, p_vertices, buffer_size);
    vkUnmapMemory(device, staging_buffer_memory);

    if (!create_buffer(buffer_size, BUFFER_TYPE_VERTEX, &p_buffer->buffer, &p_buffer->memory))
        return false;

    VkCommandBuffer command_buffer;
    allocate_command_buffer(&command_buffer);

    if (!begin_single_use_command_buffer(command_buffer))
        return false;

    VkBufferCopy buffer_copy;
    buffer_copy.size = buffer_size;
    buffer_copy.dstOffset = 0;
    buffer_copy.srcOffset = 0;

    vkCmdCopyBuffer(command_buffer, staging_buffer, p_buffer->buffer, 1, &buffer_copy);

    VkResult result = vkEndCommandBuffer(command_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to end a command buffer used for copying buffers. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    vkDestroyBuffer(device, staging_buffer, NULL);
    vkFreeMemory(device, staging_buffer_memory, NULL);

    return true;
}

void destroy_buffer(const struct buffer* p_buffer)
{
    VkDevice device = get_global_logical_device();
    vkDestroyBuffer(device, p_buffer->buffer, NULL);
    vkFreeMemory(device, p_buffer->memory, NULL);
}
