#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

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

    /* The width and the height. */
    uint16_t width;
    uint16_t height;

    /* The user pointer. */
    void* user_pointer;

    /* Callbacks. */
    window_resize_callback_func resize_callback;
};

/* A basic function to get atoms. Not an efficient method, but it works. */
static xcb_atom_t get_atom(xcb_connection_t* connection, const char* name)
{
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(connection, true, strlen(name), name);
    xcb_intern_atom_reply_t* reply =
        xcb_intern_atom_reply(connection, cookie, NULL);
    return reply->atom;
}

struct window* create_window(uint16_t width, uint16_t height, const char* title,
                             void* user_pointer)
{
    struct window* window = malloc(sizeof(struct window));
    window->width = width;
    window->height = height;

    window->user_pointer = user_pointer;
    window->resize_callback = NULL;

    window->connection = xcb_connect(NULL, NULL);

    window->atoms.WM_DELETE_WINDOW =
        get_atom(window->connection, "WM_DELETE_WINDOW");
    window->atoms.WM_PROTOCOLS = get_atom(window->connection, "WM_PROTOCOLS");

    const xcb_setup_t* setup = xcb_get_setup(window->connection);
    const xcb_screen_iterator_t screen_iterator =
        xcb_setup_roots_iterator(setup);
    xcb_screen_t* screen = screen_iterator.data;

    const uint16_t window_x_pos = (screen->width_in_pixels - width) / 2;
    const uint16_t window_y_pos = (screen->height_in_pixels - height) / 2;

    const xcb_event_mask_t event_masks[1] = {XCB_EVENT_MASK_STRUCTURE_NOTIFY};

    window->window = xcb_generate_id(window->connection);
    xcb_create_window(window->connection, XCB_COPY_FROM_PARENT, window->window,
                      screen->root, (int16_t)window_x_pos,
                      (int16_t)window_y_pos, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                      XCB_CW_EVENT_MASK, event_masks);

    /* Changes the title. */
    xcb_change_property(window->connection, XCB_PROP_MODE_REPLACE,
                        window->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        strlen(title), title);

    xcb_size_hints_t size_hints;
    xcb_icccm_size_hints_set_min_size(&size_hints, width, height);
    xcb_icccm_size_hints_set_max_size(&size_hints, width, height);
    xcb_icccm_size_hints_set_position(&size_hints, false, window_x_pos,
                                      window_y_pos);

    xcb_icccm_set_wm_size_hints(window->connection, window->window,
                                XCB_ATOM_WM_NORMAL_HINTS, &size_hints);

    xcb_map_window(window->connection, window->window);

    /* Allows us, the application, to handle Window closing events. */
    xcb_change_property(window->connection, XCB_PROP_MODE_REPLACE,
                        window->window, window->atoms.WM_PROTOCOLS, 4, 32, 1,
                        &(window->atoms.WM_DELETE_WINDOW));

    xcb_flush(window->connection);

    window->is_open = true;

    return window;
}

const char**
get_required_windowing_instance_extensions(uint32_t* extension_count)
{
    *extension_count = 2;
    const char** extensions = malloc(2 * sizeof(char*));

    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    extensions[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;

    return extensions;
}

VkSurfaceKHR create_surface_from_window(const struct window* window,
                                        VkInstance instance, bool* status)
{
    VkXcbSurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.connection = window->connection;
    create_info.window = window->window;

    VkSurfaceKHR surface;
    VkResult result =
        vkCreateXcbSurfaceKHR(instance, &create_info, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to create the XCB surface. Vulkan error %d.\n",
                result);
        *status = false;
        return surface;
    }

    *status = true;
    return surface;
}

void get_window_size(struct window* self, uint16_t* width, uint16_t* height)
{
    *width = self->width;
    *height = self->height;
}

void set_window_resize_callback(struct window* self,
                                window_resize_callback_func callback)
{
    self->resize_callback = callback;
}

bool is_window_open(struct window* window) { return window->is_open; }

void update_window(struct window* window)
{
    xcb_generic_event_t* event = xcb_poll_for_event(window->connection);

    if (event != NULL)
    {
        switch (event->response_type & ~0x80)
        {
        case XCB_CLIENT_MESSAGE:
        {
            xcb_client_message_event_t* client_message_event =
                (xcb_client_message_event_t*)event;
            if (client_message_event->data.data32[0] ==
                window->atoms.WM_DELETE_WINDOW)
            {
                // Where the window closes.
                window->is_open = false;
                return;
            }
        }
        break;
        case XCB_CONFIGURE_NOTIFY:
        {
            xcb_configure_notify_event_t* configure_notify_event =
                (xcb_configure_notify_event_t*)event;
            window->width = configure_notify_event->width;
            window->height = configure_notify_event->height;

            if (window->resize_callback)
            {
                window->resize_callback(window->user_pointer,
                                        configure_notify_event->width,
                                        configure_notify_event->height);
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
