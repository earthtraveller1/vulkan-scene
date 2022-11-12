#include <stdio.h>
#include <stdlib.h>

#include "device.h"

static VkInstance create_instance(const char* app_name)
{
    VkApplicationInfo application_info;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = app_name;
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = NULL;
    application_info.engineVersion = 0;
    application_info.apiVersion = VK_VERSION_1_2;
    
    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    
    /* TODO: These will have to be filled out eventually. */
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = NULL;
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        /* Normally, we would have a more sophisticated system for handling er-
        rors like this one, but the application has nothing else to do if the  
        instance creation fails, so it just aborts the program. */
        fprintf(stderr, "[FATAL ERROR]: Failed to create the Vulkan instance. Vulkan error %d.\n", result);
        exit(EXIT_FAILURE);
    }
    
    return instance;
}

void create_new_device(struct device* device, const char* app_name)
{
    device->instance = create_instance(app_name);
    /* TODO: Create the other objects as well. */
}