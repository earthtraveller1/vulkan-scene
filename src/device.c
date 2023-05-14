#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device.h"
#include "swapchain.h"
#include "window.h"

/* The internal objects */
static VkInstance instance;

/* The debug messenger handle. */
static VkDebugUtilsMessengerEXT debug_messenger;

/* The Window surface. This will be needed when we present
 * shit to the screen. */
static VkSurfaceKHR window_surface;

/* The handle to the physical device. This will be primarily used
 * for creating the logical device. */
static VkPhysicalDevice physical_device;

/* The queue families. */
static uint32_t graphics_queue_family, present_queue_family;

/* The logical device, which is a logical representation of the physical
 * device. This is used a lot in Vulkan. */
static VkDevice device;

/* The handles to the device queues. */
static VkQueue graphics_queue, present_queue;

/* The command pools that the command buffers are allocated from. */
static VkCommandPool command_pool;

/* The validation layers. */
const char* const VALIDATION_LAYERS[1] = {"VK_LAYER_KHRONOS_validation"};

/* The debug callback */
/* NOLINTBEGIN(bugprone-*) */
static VkBool32 VKAPI_PTR debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                   VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
/* NOLINTEND(bugprone-*) */
{
    /* Silence unused parameter warnings. */
    (void)messageTypes;
    (void)pUserData;

    /* Warnings and higher severity gets outputted to stderr */
    FILE* output_target;
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        output_target = stderr;
    else
        output_target = stdout;

    const char* color = NULL;

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        color = "90";
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        color = "90"; /* Infos reported by the debug messenger are usually useless anyways */
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        color = "93";
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        color = "91";

    fprintf(output_target, "\033[%sm[VULKAN]: %s]\033[0m\n", color, pCallbackData->pMessage);

    return VK_FALSE;
}

/* The function that creates the debug messenger create info. */
static VkDebugUtilsMessengerCreateInfoEXT get_debug_messenger_create_info(void)
{
    VkDebugUtilsMessengerCreateInfoEXT messenger;
    messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger.pNext = NULL;
    messenger.flags = 0;
    messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
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

    uint32_t glfw_extension_count;
    const char* const* const glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    uint32_t extension_count;
    const char** extensions;

    /* This is quite a bit of copying, but since it's on startup, it shouldn't
     * matter too much. */
    if (p_enable_validation)
    {
        extension_count = glfw_extension_count + 1;
        extensions = malloc(extension_count * sizeof(char*));

        /* Copy the GLFW required extensions into the extensions array. */
        memcpy(extensions, glfw_extensions, glfw_extension_count * sizeof(const char*));

        /* Insert the debug utility extension into the end of the array. */
        extensions[extension_count - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    else
    {
        extension_count = glfw_extension_count;
        extensions = malloc(extension_count * sizeof(char*));

        /* Copy the GLFW required extensions into the extensions array. */
        memcpy(extensions, glfw_extensions, glfw_extension_count * sizeof(const char*));
    }

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
    create_info.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        printf("[ERROR]: Failed to create the instance. Vulkan error %d.", result);
        return false;
    }

    free(extensions);

    return true;
}

static bool create_debug_messenger(void)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (!vkCreateDebugUtilsMessengerEXT)
    {
        fprintf(stderr, "[ERROR]: Failed to load function vkCreateDebugUtilsMessengerEXT.\n");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info = get_debug_messenger_create_info();

    VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &create_info, NULL, &debug_messenger);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create the debug messenger. Vulkan error %d", result);
        return false;
    }

    return true;
}

/* Destroying the debug messenger isn't trivial, as the function
 * needs to be manually loaded. */
static void destroy_debug_messenger(void)
{
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
    vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (vkDestroyDebugUtilsMessengerEXT)
    {
        vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
    }
}

static void find_queue_families(VkPhysicalDevice device, uint32_t* graphics_family, bool* graphics_valid, uint32_t* present_family,
                                bool* present_valid)
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

static bool physical_device_supports_swapchain(VkPhysicalDevice p_physical_device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(p_physical_device, NULL, &extension_count, NULL);

    /* This is an unlikely edge case, but just to be safe. */
    if (!extension_count)
        return false;

    VkExtensionProperties* const extensions = malloc(extension_count * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(p_physical_device, NULL, &extension_count, extensions);

    for (const VkExtensionProperties* extension = extensions; extension < extensions + extension_count; extension++)
    {
        if (strcmp(extension->extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            free(extensions);
            return true;
        }
    }

    free(extensions);
    return false;
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

    find_queue_families(p_physical_device, &graphics_family, &graphics_family_valid, &present_family, &present_family_valid);

    /* Make sure that the physical device supports all the required queue families. */
    if (!graphics_family_valid || !present_family_valid)
        return false;

    /* Make sure that hte physical device supports swap chains.
     * (Though, generally, if it supports the present queue family, it should also support swap chains) */
    if (!physical_device_supports_swapchain(p_physical_device))
        return false;

    /* Even if the physical device supports swap chains, it might not be adequate. That's why we must check the adequacy of the swap chain
     * on the physical device as well. Of course, that would be a very rare edge case, but it's still better to be safe by covering this. */

    const struct swap_chain_support_info swap_chain_support_info = get_swap_chain_support_info(p_physical_device);

    /* All we're asking for is at least one surface format and one present mode */
    if (!swap_chain_support_info.surface_format_count || !swap_chain_support_info.present_mode_count)
    {
        destroy_swap_chain_support_info(&swap_chain_support_info);
        return false;
    }

    /* Remember to destroy it, as it contains heap-allocated memory. */
    destroy_swap_chain_support_info(&swap_chain_support_info);

    /* If the device passes all of the requirements, then it is adequate. */
    return true;
}

static bool choose_physical_device(void)
{
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

    if (!physical_device_count)
    {
        fprintf(stderr, "\033[91m[ERROR]: None of your graphics cards supports Vulkan, bozo. :joy_cat: :joy_cat: :joy_cat:\n\033[0m");
        return false;
    }

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
    else
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        printf("[INFO]: Selected the %s graphics card.\n", properties.deviceName);

        bool graphics_adequate, present_adequate;
        find_queue_families(physical_device, &graphics_queue_family, &graphics_adequate, &present_queue_family, &present_adequate);

        /* Of course, if we got this far, both the graphics and present queue
         * families should be valid, so we don't need to check it. But, just in
         * case something funky happened (driver bug, maybe?), we're gonna add
         * a check anyways (Yes I am dumb and have no idea what I'm doing). */
        if (!graphics_adequate || !present_adequate)
        {
            free(physical_devices);
            return false;
        }
    }

    free(physical_devices);

    return found_adequate_device;
}

/* This creates the actual device object that is used by Vulkan,
 * not just the whole encapsulation in general. */
static bool create_vulkan_device(void)
{
    VkDeviceQueueCreateInfo* queue_create_infos;
    uint32_t queue_create_info_count;

    const float queue_priority = 1.0f;

    /* If the graphics and present family are the same, we only
     * need to create one queue, as that one queue would be in
     * both families */
    if (graphics_queue_family == present_queue_family)
    {
        queue_create_info_count = 1;
        queue_create_infos = malloc(queue_create_info_count * sizeof(VkDeviceQueueCreateInfo));

        queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].pNext = NULL;
        queue_create_infos[0].flags = 0;
        queue_create_infos[0].queueFamilyIndex = graphics_queue_family;
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queue_priority;
    }
    else
    {
        queue_create_info_count = 2;
        queue_create_infos = malloc(queue_create_info_count * sizeof(VkDeviceQueueCreateInfo));

        /* Use a constant here to make it easier for the compiler
         * to unroll this loop. */
        for (int i = 0; i < 2; i++)
        {
            queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_infos[i].pNext = NULL;
            queue_create_infos[i].flags = 0;
            queue_create_infos[i].queueCount = 1;
            queue_create_infos[i].pQueuePriorities = &queue_priority;
        }

        queue_create_infos[0].queueFamilyIndex = graphics_queue_family;
        queue_create_infos[1].queueFamilyIndex = present_queue_family;
    }

    const char* const extensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.queueCreateInfoCount = queue_create_info_count;
    create_info.pQueueCreateInfos = queue_create_infos;

    /* Technically, these don't have to be set, but I'm just
     * doing it for completeness sake. */
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = NULL;

    /* TODO: Enable swap chain extension */
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = extensions;

    create_info.pEnabledFeatures = NULL;

    VkResult result = vkCreateDevice(physical_device, &create_info, NULL, &device);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create the logical device. Vulkan error %d.\n", result);
        return false;
    }

    /* Obtain the handles to the queues that were created automatically. */
    vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, present_queue_family, 0, &present_queue);

    free(queue_create_infos);

    return true;
}

static bool create_command_pool(void)
{
    VkCommandPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.queueFamilyIndex = 0;
    create_info.queueFamilyIndex = graphics_queue_family;

    VkResult result = vkCreateCommandPool(device, &create_info, NULL, &command_pool);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create the graphics command pool. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

bool create_device(bool p_enable_validation)
{
    if (!create_instance(p_enable_validation))
        return false;

    if (p_enable_validation && !create_debug_messenger())
        return false;

    if (!get_window_surface(instance, &window_surface))
        return false;

    if (!choose_physical_device())
        return false;

    if (!create_vulkan_device())
        return false;

    if (!create_command_pool())
        return false;

    return true;
}

bool allocate_command_buffer(VkCommandBuffer* p_buffer)
{
    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(device, &allocate_info, p_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create a single use command buffer. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

VkInstance get_global_instance(void) { return instance; }

VkSurfaceKHR get_global_surface(void) { return window_surface; }

VkPhysicalDevice get_global_physical_device(void) { return physical_device; }

VkDevice get_global_logical_device(void) { return device; }

VkQueue get_global_graphics_queue(void) { return graphics_queue; }

VkQueue get_global_present_queue(void) { return present_queue; }

void destroy_device(void)
{
    vkDestroyCommandPool(device, command_pool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, window_surface, NULL);
    destroy_debug_messenger();
    vkDestroyInstance(instance, NULL);
}

void get_global_queue_families(uint32_t* graphics, uint32_t* present)
{
    *graphics = graphics_queue_family;
    *present = present_queue_family;
}
