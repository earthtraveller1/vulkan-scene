#include <Windows.h>
#include <stdexcept>
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR

#include "../../window.hpp"

// The Windows implementation of the Window class.

using vulkan_scene::Window;
using namespace std::string_literals;

struct vulkan_scene::WindowImpl
{
    HINSTANCE hInstance;
    HWND window;

    // Whether the window is open or not.
    bool is_open{true};
};

namespace
{

const wchar_t* WINDOW_CLASS = L"Vulkan Scene Window Class";

LRESULT CALLBACK window_procedure(HWND p_window, UINT p_message,
                                  WPARAM p_wide_parameter,
                                  LPARAM p_long_parameter)
{
    vulkan_scene::WindowImpl* impl = nullptr;

    if (p_message == WM_NCCREATE)
    {
        const auto create_struct =
            reinterpret_cast<CREATESTRUCT*>(p_long_parameter);
        impl = reinterpret_cast<vulkan_scene::WindowImpl*>(
            create_struct->lpCreateParams);
        SetWindowLongPtrW(p_window, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(impl));
    }
    else
    {
        const auto long_ptr = GetWindowLongPtrW(p_window, GWLP_USERDATA);
        impl = reinterpret_cast<vulkan_scene::WindowImpl*>(long_ptr);
    }

    switch (p_message)
    {
    case WM_CLOSE:
        impl->is_open = false;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(p_window, p_message, p_wide_parameter,
                              p_long_parameter);
    }
}

} // namespace

Window::Window(std::string_view p_title, uint16_t p_width, uint16_t p_height)
    : m_impl(std::make_unique<WindowImpl>())
{
    m_impl->hInstance = GetModuleHandleW(nullptr);

    const WNDCLASSW window_class{.style = 0,
                                 .lpfnWndProc = window_procedure,
                                 .hInstance = m_impl->hInstance,
                                 .hCursor =
                                     LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW),
                                 .lpszClassName = WINDOW_CLASS};

    RegisterClassW(&window_class);

    wchar_t* title = new wchar_t[p_title.size() + 1];
    size_t chars_converted;
    mbstowcs_s(&chars_converted, title, (p_title.size()) * sizeof(wchar_t),
               p_title.data(), p_title.size());

    m_impl->window =
        CreateWindowExW(0, WINDOW_CLASS, title, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, p_width, p_height,
                        nullptr, nullptr, m_impl->hInstance, m_impl.get());

    if (!m_impl->window)
    {
        throw std::runtime_error("Failed to create the window '"s +
                                 std::string(p_title) + '\'');
    }

    ShowWindow(m_impl->window, SW_NORMAL);
}

VkSurfaceKHR Window::create_surface(VkInstance p_instance) const
{
    const VkWin32SurfaceCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = m_impl->hInstance,
        .hwnd = m_impl->window};

    VkSurfaceKHR surface;
    const auto result =
        vkCreateWin32SurfaceKHR(p_instance, &create_info, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create a Vulkan window surface. Vulkan error "s +
            std::to_string(result) + '.');
    }

    return surface;
}

bool Window::is_open() const { return m_impl->is_open; }

void Window::update()
{
    MSG message;
    while (PeekMessageW(&message, m_impl->window, 0, 0, 1))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

Window::~Window() { DestroyWindow(m_impl->window); }