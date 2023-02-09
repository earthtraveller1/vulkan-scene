#include <iostream>
#include <string>

#include <xcb/xcb.h>

#ifdef _WIN32
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include "../../window.hpp"

using vulkan_scene::Window;
using namespace std::string_literals;

namespace
{
xcb_atom_t get_atom(xcb_connection_t* p_connection, std::string_view p_name)
{
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(p_connection, true, p_name.size(), p_name.data());
    xcb_intern_atom_reply_t* reply =
        xcb_intern_atom_reply(p_connection, cookie, 0);

    return reply->atom;
}
} // namespace

struct vulkan_scene::WindowImpl
{
    xcb_connection_t* connection;
    xcb_window_t window;
    bool is_open{true};

    // X11 atoms
    struct
    {
        xcb_atom_t WM_PROTOCOLS;
        xcb_atom_t WM_DELETE_WINDOW;
    } atoms;
    
    // Window dimensions
    uint16_t width;
    uint16_t height;
};

Window::Window(std::string_view p_title, uint16_t p_width, uint16_t p_height)
    : m_impl(std::make_unique<WindowImpl>())
{
    m_impl->connection = xcb_connect(nullptr, nullptr);
    m_impl->width = p_width;
    m_impl->height = p_height;

    m_impl->atoms.WM_PROTOCOLS = get_atom(m_impl->connection, "WM_PROTOCOLS");
    m_impl->atoms.WM_DELETE_WINDOW =
        get_atom(m_impl->connection, "WM_DELETE_WINDOW");

    const xcb_setup_t* setup = xcb_get_setup(m_impl->connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t* screen = iter.data;

    m_impl->window = xcb_generate_id(m_impl->connection);
    xcb_create_window(m_impl->connection, XCB_COPY_FROM_PARENT, m_impl->window,
                      screen->root, 0, 0, p_width, p_height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0,
                      nullptr);

    xcb_change_property(m_impl->connection, XCB_PROP_MODE_REPLACE,
                        m_impl->window, m_impl->atoms.WM_PROTOCOLS, 4, 32, 1,
                        &(m_impl->atoms.WM_DELETE_WINDOW));

    xcb_change_property(m_impl->connection, XCB_PROP_MODE_REPLACE,
                        m_impl->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        p_title.size(), p_title.data());

    xcb_map_window(m_impl->connection, m_impl->window);

    xcb_flush(m_impl->connection);
}

VkSurfaceKHR Window::create_surface(VkInstance p_instance) const
{
    const VkXcbSurfaceCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .connection = m_impl->connection,
        .window = m_impl->window};

    VkSurfaceKHR surface;
    const auto result =
        vkCreateXcbSurfaceKHR(p_instance, &create_info, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create a Vulkan surface. Vulkan error "s +
            std::to_string(result) + '.');
    }

    return surface;
}

bool Window::is_open() const { return m_impl->is_open; }

uint16_t Window::get_width() const { return m_impl->width; }

uint16_t Window::get_height() const { return m_impl->height; }

void Window::update()
{
    const auto event = xcb_poll_for_event(m_impl->connection);

    if (event)
    {
        switch (event->response_type & ~0x80)
        {
        case XCB_CLIENT_MESSAGE:
        {
            const auto client_message_event =
                reinterpret_cast<xcb_client_message_event_t*>(event);
            if (client_message_event->data.data32[0] ==
                m_impl->atoms.WM_DELETE_WINDOW)
            {
                // TODO: Close the window.
                m_impl->is_open = false;
            }
        }
        }
    }
}

Window::~Window() { xcb_disconnect(m_impl->connection); }
