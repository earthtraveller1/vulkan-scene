#ifndef INCLUDED_DEVICE_H
#define INCLUDED_DEVICE_H

#include <vulkan/vulkan.h>

/*
So, a basic device encapsulation system. It's meant to encapsulate all of the  
singleton objects within this context.
*/

struct device
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;
};

void create_new_device(struct device* device, const char* app_name);

void destroy_device(struct device* device);

#endif