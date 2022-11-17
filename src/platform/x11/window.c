#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb.h>

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

#include "../../window.h"

struct x11_atoms
{
    xcb_atom_t WM_PROTOCOLS;
    xcb_atom_t WM_DELETE_WINDOW;
};

struct window
{
    xcb_connection_t* connection;
    xcb_window_t window;
    bool is_open;
    
    /* X11 Atoms */
    struct x11_atoms atoms;
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
    
    /* Allows us, the application, to handle Window closing events. */
    {
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(window->connection, true, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(window->connection, cookie, NULL);
        
        xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(window->connection, true, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
        xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(window->connection, cookie2, NULL);
        
        xcb_change_property(window->connection, XCB_PROP_MODE_REPLACE, window->window, reply->atom, 4, 32, 1, &(reply2->atom));
        
        xcb_flush(window->connection);
        
        /* We'll need some of these atoms later. */
        window->atoms.WM_PROTOCOLS = reply->atom;
        window->atoms.WM_DELETE_WINDOW = reply2->atom;
    }
    
    window->is_open = true;
    
    return window;
}

const char** get_required_windowing_instance_extensions(uint32_t* extension_count)
{
    const char** extensions = malloc(2 * sizeof(char*));
    
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
        fprintf(stderr, "[ERROR]: Failed to create the XCB surface. Vulkan error %d.\n", result);
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

void update_window(struct window* window)
{
    xcb_generic_event_t* event = xcb_poll_for_event(window->connection);
    
    if (event != NULL)
    {
        switch (event->response_type & ~0x80)
        {
        case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* client_message_event = (xcb_client_message_event_t*)event;
                if (client_message_event->data.data32[0] == window->atoms.WM_DELETE_WINDOW)
                {
                    // Where the window closes.
                    window->is_open = false;
                    return;
                }
            }
            break;
        }
    }
}

void destroy_window(struct window* window)
{
    xcb_disconnect(window->connection);
    free(window);
}