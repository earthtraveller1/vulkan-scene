#include <stdio.h>

#include "commands.h"

bool create_new_command_pool(const struct device* device, struct command_pool* self)
{
    self->device = device;
    
    VkCommandPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.queueFamilyIndex = device->graphics_queue_family;
    
    VkResult result = vkCreateCommandPool(device->device, &create_info, NULL, &self->command_pool);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a command pool. Vulkan error %d.\n", result);
        return false;
    }
    
    return true;
}

bool create_new_command_buffer(const struct command_pool* self, VkCommandBuffer* command_buffer)
{
    VkCommandBufferAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.commandPool = self->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    
    VkResult result = vkAllocateCommandBuffers(self->device->device, &alloc_info, command_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to allocate command buffer. Vulkan error %d.\n", result);
        return false;
    }
    
    return true;
}