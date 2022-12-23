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

bool create_vulkan_fence(const struct device* device, VkFence* fence)
{
    VkFenceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = NULL;
    
    /* This is to prevent the first frame from being blocked indefinitely. */
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VkResult result = vkCreateFence(device->device, &create_info, NULL, fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a Vulkan fence. Vulkan error %d.\n", result);
        return false;
    }
    
    return true;
}