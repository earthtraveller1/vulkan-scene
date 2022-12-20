#include <stdbool.h>
#include <stdio.h>

#include "device.h"

#include "synchronization.h"

bool create_vulkan_semaphore(const struct device* device, VkSemaphore* semaphore)
{
    VkSemaphoreCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    
    VkResult result = vkCreateSemaphore(device->device, &create_info, NULL, semaphore);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a Vulkan semaphore. Vulkan error %d.\n", result);
        return false;
    }
    
    return true;
}