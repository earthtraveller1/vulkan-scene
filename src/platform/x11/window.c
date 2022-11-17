#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

#include "../../window.h"

struct window
{
    xcb_connection_t* connection;
    xcb_window_t window;
    bool is_open;
};

struct window* create_window(uint16_t width, uint16_t height, const char* title)
{
    struct window* window = malloc(sizeof(struct window));
    
    window->connection = xcb_connect(NULL, NULL);
    
    const xcb_setup_t* setup = xcb_get_setup(window->connection);
    const xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);
    xcb_screen_t* screen = screen_iterator.data;
    
    const int16_t window_x_pos = (screen->width_in_pixels - width) / 2;
    const int16_t window_y_pos = (screen->height_in_pixels - height) / 2;
    
    window->window = xcb_generate_id(window->connection);
    xcb_create_window(
        window->connection, 
        XCB_COPY_FROM_PARENT, 
        window->window, 
        screen->root, 
        window_x_pos, window_y_pos, 
        width, height, 
        0, 
        XCB_WINDOW_CLASS_INPUT_OUTPUT, 
        screen->root_visual, 
        0, NULL
    );
    
    window->is_open = true;
    
    return window;
}

const char** get_required_windowing_instance_extensions(uint32_t* extension_count)
{
    char** extensions = malloc(2 * sizeof(char*));
    
    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    extensions[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
    
    return extensions;
}

VkSurfaceKHR create_surface_from_window(struct window* window, VkInstance instance, bool* status)
{
    VkXcbSurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.connection = window->connection;
    create_info.window = window->window;
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateXcbSurfaceKHR(instance, &create_info, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf("[ERROR]: Failed to create the XCB surface. Vulkan error %d.\n", result);
        *status = false;
        return surface;
    }
    
    *status = true;
    return surface;
}

void show_window(struct window* window)
{
    xcb_map_window(window->connection, window->window);
    xcb_flush(window->connection);
}

bool is_window_open(struct window* window)
{
    return window->is_open;
}