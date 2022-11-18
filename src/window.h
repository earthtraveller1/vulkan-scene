#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

/* A basic window abstraction. Doesn't do much besides what I need for this pr-
oject in particular */

struct window;

/* Creates a window. */
struct window* create_window(uint16_t width, uint16_t height, const char* title);

/* Returns the required extensions for windowing to integrate with Vulkan. */
const char** get_required_windowing_instance_extensions(uint32_t* extension_count);

/* Creates a Vulkan surface from this window. */
VkSurfaceKHR create_surface_from_window(const struct window* window, VkInstance instance, bool* status);

/* Checks if the window is still open. */
bool is_window_open(struct window* window);

/* Updates the window. */
void update_window(struct window* window);

/* Destroys the window. Please note that window pointer is no longer valid aft-
er this is called, as it has been freed. */
void destroy_window(struct window* window);

#endif