#include <stdio.h>
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
