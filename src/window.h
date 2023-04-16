#ifndef INCLUDED_window_h
#define INCLUDED_window_h

#include <stdbool.h>
#include <vulkan/vulkan.h>

/* This file basically contains all of the windowing-related stuff. */

bool create_window();

VkSurfaceKHR get_window_surface(VkInstance p_instance);

bool is_window_open();

void update_window();

void destroy_window();

#endif
