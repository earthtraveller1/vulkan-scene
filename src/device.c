#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "vk-ext.h"
#include "window.h"

#include "device.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

/* Please don't mess up the order in which the fields are defined. Thanks in a-
dvance. */
static const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO = {
    /* sType = */ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    /* pNext = */ NULL,
    /* flags = */ 0,
    /* messageSeverity = */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
#ifdef VERBOSE_VULKAN_DEBUG_LOGGING
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
#endif
    ,
    /* messageType = */ VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
    /* pfnUserCallback = */ debug_messenger_callback,
    /* pUserData = */ NULL};

/* Number of required device extensions. */
#define REQUIRED_DEVICE_EXTENSION_COUNT 1

typedef const char* Str;

/* Platform independent extensions. */
static const Str REQUIRED_DEVICE_EXTENSIONS[REQUIRED_DEVICE_EXTENSION_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/* This is so that we can insert a breakpoint just before the debug callback r-
eturns. */
static void do_nothing() {}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
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
    else
    {
        printf("[VULKAN]: %s\n", callback_data->pMessage);
    }

    /* Insert a breakpoint here. */
    do_nothing();

    return VK_FALSE;
}

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
    const char** windowing_extensions =
        get_required_windowing_instance_extensions(&window_extension_count);

    const char** enabled_extension_names =
        malloc(window_extension_count * sizeof(const char*));

    for (uint32_t i = 0; i < window_extension_count; i++)
    {
        enabled_extension_names[i] = windowing_extensions[i];
    }

    create_info.enabledExtensionCount = window_extension_count;

    if (enable_validation)
    {

        enabled_layer_names[0] = "VK_LAYER_KHRONOS_validation";
        create_info.enabledLayerCount = 1;

        enabled_extension_names =
            realloc(enabled_extension_names,
                    (window_extension_count + 1) * sizeof(const char*));
        enabled_extension_names[window_extension_count] =
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
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
        fprintf(stderr,
                "[FATAL ERROR]: Failed to create the Vulkan instance. Vulkan "
                "error %d.\n",
                result);
        exit(EXIT_FAILURE);
    }

    free(enabled_layer_names);
    free(enabled_extension_names);
    free(windowing_extensions);

    return instance;
}

static VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance,
                                                       bool* status)
{
    /* It is assumed to succeed unless otherwise. */
    *status = true;

    VkDebugUtilsMessengerEXT messenger;
    VkResult result = vkCreateDebugUtilsMessengerEXT(
        instance, &DEBUG_MESSENGER_CREATE_INFO, NULL, &messenger);
    if (result != VK_SUCCESS)
    {
        fprintf(
            stderr,
            "[ERROR]: Failed to create the debug messenger. Vulkan error %d.\n",
            result);
        *status = false;
    }

    return messenger;
}

static void get_queue_families(VkPhysicalDevice physical_device,
                               VkSurfaceKHR surface, uint32_t* graphics_family,
                               uint32_t* present_family, bool* graphics_valid,
                               bool* present_valid)
{
    /* They are presumed to be invalid until proven valid. */
    *graphics_valid = false;
    *present_valid = false;

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             &queue_family_count, NULL);

    VkQueueFamilyProperties* queue_families =
        malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_families);

    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            *graphics_valid = true;
            *graphics_family = i;
        }

        VkBool32 can_present;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &can_present);
        if (can_present)
        {
            *present_valid = true;
            *present_family = i;
        }
    }

    free(queue_families);
}

/* Checks if the physical device supports the required device extensions. */
static bool
does_device_support_required_extensions(VkPhysicalDevice physical_device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, NULL,
                                         &extension_count, NULL);

    VkExtensionProperties* extensions =
        malloc(extension_count * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(physical_device, NULL,
                                         &extension_count, extensions);

    const char* unsupported_extensions[REQUIRED_DEVICE_EXTENSION_COUNT];

    for (uint32_t i = 0; i < REQUIRED_DEVICE_EXTENSION_COUNT; i++)
    {
        unsupported_extensions[i] = REQUIRED_DEVICE_EXTENSIONS[i];
    }

    for (uint32_t i = 0; i < REQUIRED_DEVICE_EXTENSION_COUNT; i++)
    {
        for (uint32_t j = 0; j < extension_count; j++)
        {
            if (strcmp(unsupported_extensions[i],
                       extensions[j].extensionName) == 0)
            {
                unsupported_extensions[i] = NULL;
                break;
            }
        }
    }

    free(extensions);

    bool result = true;

    /* Now check if there are no unsupported extensions. */
    for (const char** extension = unsupported_extensions;
         extension < unsupported_extensions + REQUIRED_DEVICE_EXTENSION_COUNT;
         extension++)
    {
        if (*extension)
        {
            result = false;
        }
    }

    return result;
}

static VkPhysicalDevice choose_physical_device(VkInstance instance,
                                               VkSurfaceKHR surface,
                                               uint32_t* graphics_family,
                                               uint32_t* present_family,
                                               bool* found_usable_device)
{
    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);

    VkPhysicalDevice* physical_devices =
        malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices);

    VkPhysicalDevice chosen_device = VK_NULL_HANDLE;

    /* For now, we just choose the first device that fits our requirements that
    is NOT a CPU renderer. */
    for (VkPhysicalDevice* device = physical_devices;
         device < physical_devices + device_count; device++)
    {
        bool graphics_valid, present_valid;

        get_queue_families(*device, surface, graphics_family, present_family,
                           &graphics_valid, &present_valid);

        if (graphics_valid && present_valid &&
            does_device_support_required_extensions(*device))
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
        fputs("[ERROR]: Unable to find a usable device.", stderr);
    }
    else
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(chosen_device, &device_properties);

        printf("[INFO]: Using the %s graphics card.\n",
               device_properties.deviceName);

        printf(
            "[INFO]: %d bytes is the maximum size for push constants for %s.\n",
            device_properties.limits.maxPushConstantsSize,
            device_properties.deviceName);
    }

    free(physical_devices);

    return chosen_device;
}

static void create_vulkan_device(struct device* device, bool* status)
{
    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;

    const float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo* queue_create_infos;

    if (device->graphics_queue_family == device->present_queue_family)
    {
        /* We just create one queue if the queue families are the same. */
        create_info.queueCreateInfoCount = 1;

        queue_create_infos = malloc(sizeof(VkDeviceQueueCreateInfo));
        queue_create_infos[0].sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].pNext = NULL;
        queue_create_infos[0].flags = 0;
        queue_create_infos[0].queueFamilyIndex =
            device->graphics_queue_family; /* It doesn't really matter which one
                                              since they're both the same. */
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queue_priority;
    }
    else
    {
        create_info.queueCreateInfoCount = 2;

        queue_create_infos = malloc(create_info.queueCreateInfoCount *
                                    sizeof(VkDeviceQueueCreateInfo));

        uint32_t queue_families[2] = {device->graphics_queue_family,
                                      device->present_queue_family};

        for (uint8_t i = 0; i < 2; i++)
        {
            queue_create_infos[i].sType =
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_infos[i].pNext = NULL;
            queue_create_infos[i].flags = 0;
            queue_create_infos[i].queueFamilyIndex = queue_families[i];
            queue_create_infos[i].queueCount = 1;
            queue_create_infos[i].pQueuePriorities = &queue_priority;
        }
    }

    create_info.pQueueCreateInfos = queue_create_infos;

    /* Although these are deprecated, in some cases they might be accessed so
    we need to specify them anyways. */
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;

    /* We don't have any platform-dependent device extensions yet. */
    create_info.enabledExtensionCount = REQUIRED_DEVICE_EXTENSION_COUNT;
    create_info.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS;

    /* We aren't using any device features but might enable them in the future.
     */
    create_info.pEnabledFeatures = NULL;

    VkResult result = vkCreateDevice(device->physical_device, &create_info,
                                     NULL, &(device->device));
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[FATAL ERROR]: Failed to create the logical device. Vulkan "
                "error %d.\n",
                result);
        *status = false;
        return;
    }

    free(queue_create_infos);

    *status = true;
    return;
}

bool create_new_device(struct device* device, const char* app_name,
                       bool enable_validation, const struct window* window)
{
    device->instance = create_instance(app_name, enable_validation);
    device->enable_validation = enable_validation;

    if (enable_validation)
    {
        bool status = false;

        device->debug_messenger =
            create_debug_messenger(device->instance, &status);

        if (!status)
        {
            device->enable_validation = false;
            puts("Disabling validation.");
        }
    }

    bool status;

    device->surface =
        create_surface_from_window(window, device->instance, &status);

    if (!status)
    {
        return false;
    }

    device->physical_device = choose_physical_device(
        device->instance, device->surface, &(device->graphics_queue_family),
        &(device->present_queue_family), &status);
    if (!status)
    {
        return false;
    }

    create_vulkan_device(device, &status);
    if (!status)
    {
        return false;
    }

    vkGetDeviceQueue(device->device, device->graphics_queue_family, 0,
                     &(device->graphics_queue));
    vkGetDeviceQueue(device->device, device->present_queue_family, 0,
                     &(device->present_queue));

    if (!create_new_command_pool(device, &device->command_pool))
    {
        fputs("[ERROR]: Failed to create the command pool for the device.\n",
              stderr);
        return false;
    }

    return true;
}

void device_wait_idle(struct device* device)
{
    vkDeviceWaitIdle(device->device);
}

void destroy_device(struct device* device)
{
    destroy_command_pool(&device->command_pool);
    vkDestroyDevice(device->device, NULL);
    vkDestroySurfaceKHR(device->instance, device->surface, NULL);
    if (device->enable_validation)
    {
        vkDestroyDebugUtilsMessengerEXT(device->instance,
                                        device->debug_messenger, NULL);
    }
    vkDestroyInstance(device->instance, NULL);
}
