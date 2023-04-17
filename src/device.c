#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.h"
#include "window.h"

/* The internal objects */ 
static VkInstance instance;

/* The Window surface. This will be needed when we present
 * shit to the screen. */
static VkSurfaceKHR window_surface;

/* Creates the instance specifically. */ 
static bool create_instance()
{
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Vulkan Scene";
    app_info.applicationVersion = 0;
    app_info.pEngineName = NULL;
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_API_VERSION_1_2;

    uint32_t extension_count;
    const char* const* const glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
    
    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

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
        return false;
    
    if (!get_window_surface(instance, &window_surface))
        return false;

    return true;
}

void destroy_device()
{
    vkDestroyInstance(instance, NULL);
    vkDestroySurfaceKHR(instance, window_surface, NULL);
}
