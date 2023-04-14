#include <stdio.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "device.h"

/* The internal objects */ 
static VkInstance instance;

/* Creates the instance specifically. */ 
static bool create_instance()
{
    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.pApplicationInfo = NULL; /* TODO */ 
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = NULL;

    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        printf("[ERROR]: Failed to create the instance. Vulkan error %d.", result);
        return false;
    }

    return true;
}

bool create_device()
{
    if (!create_instance())
    {
        return false;
    }

    return true;
}

void destroy_device()
{
    vkDestroyInstance(instance, NULL);
}
