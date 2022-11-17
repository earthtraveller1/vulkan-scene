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
    const char** enabled_extension_names = malloc(1 * sizeof(const char*));
    
    if (enable_validation)
    {
        enabled_layer_names[0] = "VK_LAYER_KHRONOS_validation";
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = enabled_layer_names;
        
        enabled_extension_names[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = enabled_extension_names;
    }
    
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
    
    /* TODO: Create the other objects as well. */
}

void destroy_device(struct device* device)
{
    vkDestroyInstance(device->instance, NULL);
    
    if (device->enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(device->instance, device->debug_messenger, NULL);
    }
}
