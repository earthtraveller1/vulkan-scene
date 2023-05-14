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
VkInstance get_global_instance(void);

/* Getters for the handles. */
VkSurfaceKHR get_global_surface(void);
VkPhysicalDevice get_global_physical_device(void);
VkDevice get_global_logical_device(void);
VkQueue get_global_graphics_queue(void);
VkQueue get_global_present_queue(void);

/* Allocate a primary command buffer. */
bool allocate_command_buffer(VkCommandBuffer* p_buffer);

/* Obtain the graphics and present queue families */
void get_global_queue_families(uint32_t* graphics, uint32_t* present);

/* Destroys all of the global objects */
void destroy_device(void);

#endif
