#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>

int main()
{
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    app_info.pNext = NULL;
    app_info.pApplicationName = "Vulkan Scene";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = NULL;
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_API_VERSION_1_2;
    
    VkInstanceCreateInfo instance_info;
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = NULL;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = 0;
    instance_info.ppEnabledLayerNames = NULL;
    instance_info.enabledExtensionCount = 0;
    instance_info.ppEnabledExtensionNames = NULL;
    
    VkInstance instance;
    
    if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS)
    {
        puts("[FATAL ERROR]: Failed to create the instance.");
        return EXIT_FAILURE;
    }
    
    vkDestroyInstance(instance, NULL);
    
    return EXIT_SUCCESS;
}