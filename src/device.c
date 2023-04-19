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

/* The validation layers. */
const char* const VALIDATION_LAYERS[1] = {
    "VK_LAYER_KHRONOS_validation"
};

/* The debug callback */
static VkBool32 VKAPI_PTR debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData)
{
    /* Warnings and higher severity gets outputted to stderr */
    FILE* output_target;
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        output_target = stderr;
    else
        output_target = stdout;

    fprintf(output_target, "[VULKAN]: %s]\n", pCallbackData->pMessage);

    return VK_FALSE;
}

/* The function that creates the debug messenger create info. */
static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info()
{
    VkDebugUtilsMessengerCreateInfoEXT messenger;
    messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger.pNext = NULL;
    messenger.flags = 0;
    messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    messenger.messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    messenger.pfnUserCallback = debug_messenger_callback;
    messenger.pUserData = NULL;

    return messenger;
}

/* Creates the instance specifically. */ 
static bool create_instance(bool p_enable_validation)
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

    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = get_debug_messenger_create_info();

    if (p_enable_validation)
        create_info.pNext = &messenger_create_info;
    else
        create_info.pNext = NULL;

    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;

    if (p_enable_validation)
    {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
    }
    else 
    {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = NULL;
    }

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
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(p_physical_device, &device_properties);

    /* We don't wanna use any software versions of Vulkan. */
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        return false;

    bool graphics_family_valid, present_family_valid;
    uint32_t graphics_family, present_family;

    find_queue_families(p_physical_device, &graphics_family, &present_family, &graphics_family_valid, &present_family_valid);
    
    /* Make sure that the physical device supports all the required queue families. */
    if (!graphics_family_valid || !present_family_valid)
        return false;

    /* If the device passes all of the requirements, then it is adequate. */
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

bool create_device(bool p_enable_validation)
{
    if (!create_instance(p_enable_validation))
        return false;
    
    if (!get_window_surface(instance, &window_surface))
        return false;

    if (!choose_physical_device())
        return false;

    return true;
}

void destroy_device()
{
    vkDestroySurfaceKHR(instance, window_surface, NULL);
    vkDestroyInstance(instance, NULL);
}
