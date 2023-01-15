#include <xcb/xcb.h>

#include "../../window.hpp"

using vulkan_scene::Window;
using vulkan_scene::WindowImpl;

struct WindowImpl
{
    xcb_connection_t* connection;
    xcb_window_t window;
};

Window::Window(std::string_view p_title, uint16_t p_width, uint16_t p_height)
    : m_impl(std::make_unique<WindowImpl>())
{
    m_impl->connection = xcb_connect(nullptr, nullptr);

    const xcb_setup_t* setup = xcb_get_setup(m_impl->connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t* screen = iter.data;

    m_impl->window = xcb_generate_id(m_impl->connection);
    xcb_create_window(m_impl->connection, XCB_COPY_FROM_PARENT, m_impl->window,
                      screen->root, 0, 0, p_width, p_height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0,
                      nullptr);

    xcb_map_window(m_impl->connection, m_impl->window);

    xcb_flush(m_impl->connection);
}

bool Window::is_open() const { return true; }

void Window::update() {}

Window::~Window() { xcb_disconnect(m_impl->connection); }
