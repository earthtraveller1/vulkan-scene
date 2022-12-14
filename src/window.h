#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

/* A basic window abstraction. Doesn't do much besides what I need for this pr-
oject in particular */

struct window;

typedef void (*window_resize_callback_func)(void* user_pointer, uint16_t width,
                                            uint16_t height);

/* Creates a window. */
struct window* create_window(uint16_t width, uint16_t height, const char* title,
                             void* user_pointer);

/* Returns the required extensions for windowing to integrate with Vulkan. */
const char**
get_required_windowing_instance_extensions(uint32_t* extension_count);

/* Creates a Vulkan surface from this window. */
VkSurfaceKHR create_surface_from_window(const struct window* window,
                                        VkInstance instance, bool* status);

/**
 * \brief Obtains the width and height of the window.
 *
 * \param self The window to obtain the dimensions from.
 * \param width The address to store the width.
 * \param height The address to store the height.
 */
void get_window_size(struct window* self, uint16_t* width, uint16_t* height);

/* Sets the window resize callback. */
void set_window_resize_callback(struct window* self,
                                window_resize_callback_func callback);

/* Checks if the window is still open. */
bool is_window_open(struct window* window);

/* Updates the window. */
void update_window(struct window* window);

/* Destroys the window. Please note that window pointer is no longer valid aft-
er this is called, as it has been freed. */
void destroy_window(struct window* window);

#endif