#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "vk_ext.h"
#include "window.h"
#include "utils.h"

#include "device.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    UNUSED(message_types);
    UNUSED(user_data);
    
    if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            fprintf(stderr, "[VULKAN ERROR]: %s\n", callback_data->pMessage);
        }
        else 
        {
            fprintf(stdout, "[VULKAN WARNING]: %s\n", callback_data->pMessage);
        }
    }
    
    return VK_FALSE;
}

/* Please don't mess up the order in which the fields are defined. Thanks in a-
dvance. */
static const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO = {
    /* sType = */ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    /* pNext = */ NULL,
    /* flags = */ 0,
    /* messageSeverity = */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
    /* messageType = */ VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
    /* pfnUserCallback = */ debug_messenger_callback,
    /* pUserData = */ NULL
};

static VkInstance create_instance(const char* app_name, bool enable_validation)
{
    VkApplicationInfo application_info;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext = NULL;
    application_info.pApplicationName = app_name;
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = NULL;
    application_info.engineVersion = 0;
    application_info.apiVersion = VK_API_VERSION_1_2;
    
    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    if (enable_validation)
    {
        create_info.pNext = &DEBUG_MESSENGER_CREATE_INFO;
    }
    else 
    {
        create_info.pNext = NULL;
    }
    
    create_info.flags = 0;
    create_info.pApplicationInfo = &application_info;
    
    /* TODO: These will have to be filled out eventually. */
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = NULL;
    
    const char** enabled_layer_names = malloc(1 * sizeof(const char*));
    
    uint32_t window_extension_count;
    const char** windowing_extensions = get_required_windowing_instance_extensions(&window_extension_count);
    
    const char** enabled_extension_names = malloc(window_extension_count * sizeof(const char*));
    
    for (uint16_t i = 0; i < window_extension_count; i++)
    {
        enabled_extension_names[i] = windowing_extensions[i];
    }
    
    create_info.enabledExtensionCount = window_extension_count;
    
    if (enable_validation)
    {
        
        enabled_layer_names[0] = "VK_LAYER_KHRONOS_validation";
        create_info.enabledLayerCount = 1;
        
        enabled_extension_names = realloc(enabled_extension_names, (window_extension_count + 1) * sizeof(const char*));
        enabled_extension_names[window_extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        create_info.enabledExtensionCount = window_extension_count + 1;
    }
    
    create_info.ppEnabledLayerNames = enabled_layer_names;
    create_info.ppEnabledExtensionNames = enabled_extension_names;
    
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
    
    free(enabled_layer_names);
    free(enabled_extension_names);
    free(windowing_extensions);
    
    return instance;
}

static VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance, bool* status)
{
    /* It is assumed to succeed unless otherwise. */
    *status = true;
    
    VkDebugUtilsMessengerEXT messenger;
    VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &DEBUG_MESSENGER_CREATE_INFO, NULL, &messenger);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create the debug messenger. Vulkan error %d.\n", result);
        *status = false;
    }
    
    return messenger;
}

static void get_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface, uint32_t* graphics_family, uint32_t* present_family, bool* graphics_valid, bool* present_valid)
{
    /* They are presumed to be invalid until proven valid. */
    *graphics_valid = false;
    *present_valid = false;
    
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    
    VkQueueFamilyProperties* queue_families = malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    
    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            *graphics_valid = true;
            *graphics_family = i;
        }
        
        VkBool32 can_present;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &can_present);
        if (can_present)
        {
            *present_valid = true;
            *present_family = i;
        }
    }
    
    free(queue_families);
}

static VkPhysicalDevice choose_physical_device(VkInstance instance, VkSurfaceKHR surface, uint32_t* graphics_family, uint32_t* present_family, bool* found_usable_device)
{
    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    
    VkPhysicalDevice* physical_devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);
    
    VkPhysicalDevice chosen_device = VK_NULL_HANDLE;
    
    /* For now, we just choose the first device that fits our requirements that
    is NOT a CPU renderer. */
    for (VkPhysicalDevice* device = physical_devices; device < physical_devices + device_count; device++)
    {
        bool graphics_valid, present_valid;
        
        get_queue_families(*device, surface, graphics_family, present_family, &graphics_valid, &present_valid);
        
        if (graphics_valid && present_valid)
        {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(*device, &device_properties);
            
            if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU)
            {
                *found_usable_device = true;
                chosen_device = *device;
            }
            
            break;
        }
    }
    
    /* If we weren't able to find a usable device. */
    if (chosen_device == VK_NULL_HANDLE)
    {
        fputs(stderr, "[ERROR]: Unable to find a usable device.");
    }
    else 
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(chosen_device, &device_properties);
        
        printf("[INFO]: Using the %s graphics card.\n", device_properties.deviceName);
    }
    
    free(physical_devices);
    
    return chosen_device;
}

void create_new_device(struct device* device, const char* app_name, bool enable_validation, struct window* window, bool* status)
{
    device->instance = create_instance(app_name, enable_validation);
    device->enable_validation = enable_validation;
    
    if (enable_validation)
    {
        device->debug_messenger = create_debug_messenger(device->instance, status);
        
        if (!(*status))
        {
            device->enable_validation = false;
            puts("Disabling validation.");
        }
        
        *status = true;
    }
    
    device->surface = create_surface_from_window(window, device->instance, status);
    
    if (!(*status))
    {
        return;
    }
    
    uint32_t graphics_family, present_family;
    device->physical_device = choose_physical_device(device->instance, device->surface, &graphics_family, &present_family, status);
    if (!(*status))
    {
        return;
    }
    
    /* TODO: Create the other objects as well. */
}

void destroy_device(struct device* device)
{
    if (device->enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(device->instance, device->debug_messenger, NULL);
    }
    vkDestroyInstance(device->instance, NULL);
}
