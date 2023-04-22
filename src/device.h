#ifndef INCLUDED_device_h
#define INCLUDED_device_h

#include <stdbool.h>

#include <vulkan/vulkan.h>

/* A basic abstraction around common Vulkan device objects,
 * such as the instance, the physical device, the logical device, etc.
 */

/* Creates all of the global objects */
bool create_device(bool p_enable_validation);

/* Obtains the handle to the instance. */
VkInstance get_global_instance();

/* Getters for the handles. */
VkSurfaceKHR get_global_surface();
VkPhysicalDevice get_global_physical_device();
VkDevice get_global_logical_device();
VkQueue get_global_graphics_queue();
VkQueue get_global_present_queue();

/* Destroys all of the global objects */
void destroy_device();

#endif
