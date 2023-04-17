#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.h"
#include "window.h"

/* The internal objects */ 
static VkInstance instance;

/* The Window surface. This will be needed when we present
 * shit to the screen. */
static VkSurfaceKHR window_surface;

/* The handle to the physical device. This will be primarily used
 * for creating the logical device. */
static VkPhysicalDevice physical_device;

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

static void find_queue_families(VkPhysicalDevice device, uint32_t* graphics_family, uint32_t* present_family, bool* graphics_valid, bool* present_valid)
{
    /* We assume that we will fail until we actually succeeded */
    *graphics_valid = false;
    *present_valid = false;

    uint32_t family_count; /* The number of queue families */
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, NULL);

    if (!family_count)
        return;

    VkQueueFamilyProperties* const families = malloc(family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families);

    for (unsigned int i = 0; i < family_count; i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            *graphics_family = i;
            *graphics_valid = true;
        }

        /* Source: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface */
        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window_surface, &present_support);

        if (present_support)
        {
            *present_family = i;
            *present_valid = true;
        }
    }

    free(families);
}

static bool is_physical_device_adequate(VkPhysicalDevice p_physical_device)
{
    /* TODO: Actually check physical device adequacy */
    return true;
}

static bool choose_physical_device()
{
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

    VkPhysicalDevice* const physical_devices = malloc(physical_device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

    bool found_adequate_device = false;

    for (const VkPhysicalDevice* device = physical_devices; device < physical_devices + physical_device_count; device++)
    {
        /* Basically, just use the first device that was found to be adequate */
        if (is_physical_device_adequate(*device))
        {
            physical_device = *device;
            found_adequate_device = true;

            /* We don't want to keep looping after we've already found an adequate device */
            break;
        }
    }

    if (!found_adequate_device)
    {
        fprintf(stderr, "[ERROR]: Failed to find an adequate physical device.\n");
    }

    free(physical_devices);

    return found_adequate_device;
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
