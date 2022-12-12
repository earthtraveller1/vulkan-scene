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