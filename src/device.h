#ifndef INCLUDED_DEVICE_H
#define INCLUDED_DEVICE_H

#include <stdbool.h>

#include <vulkan/vulkan.h>

#include "commands.h"
#include "window.h"

/*
So, a basic device encapsulation system. It's meant to encapsulate all of the
singleton objects within this context.
*/

struct device
{
    /* The Vulkan instance. */
    VkInstance instance;

    /* Debugging related stuff. */
    bool enable_validation;
    VkDebugUtilsMessengerEXT debug_messenger;

    /* The actual Vulkan devices. */
    VkPhysicalDevice physical_device;
    VkDevice device;

    /* Queue families */
    uint32_t graphics_queue_family;
    uint32_t present_queue_family;

    /* The actual handles to the queues. */
    VkQueue graphics_queue;
    VkQueue present_queue;

    /* The command pool. */
    struct command_pool command_pool;

    /* Presentation related stuff. */
    VkSurfaceKHR surface;
};

bool create_new_device(struct device* device, const char* app_name,
                       bool enable_validation, const struct window* window);

#define get_command_pool_from_device(device) (&(device)->command_pool)

/* Waits for the device to finish operations. */
void device_wait_idle(struct device* device);

void destroy_device(struct device* device);

#endif