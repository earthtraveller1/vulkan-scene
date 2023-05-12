#ifndef INCLUDED_window_h
#define INCLUDED_window_h

#include <stdbool.h>
#include <vulkan/vulkan.h>

/* This file basically contains all of the windowing-related stuff. */

bool create_window(void);

bool get_window_surface(VkInstance instance, VkSurfaceKHR* surface);

bool is_window_open(void);

void get_framebuffer_size(int* width, int* height);

void update_window(void);

void destroy_window(void);

#endif
