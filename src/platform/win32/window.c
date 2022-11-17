#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../../window.h"

/* TODO: Implement the actual functions. */

struct window
{
    HINSTANCE h_instance; /* May be required for some things, but not completely sure. */
    bool is_open;
    HWND window;
};

const LPCWSTR WINDOW_CLASS_NAME = L"Vulkan Scene Window Class";

static LRESULT CALLBACK window_procedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    struct window* window;
    if (uMsg == WM_CREATE)
    {
        window = (struct window*)(((CREATESTRUCTW*)lParam)->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else
    {
        window = (struct window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    }
    
    switch (uMsg)
    {
        case WM_DESTROY:
            window->is_open = false;
            return 0;
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

struct window* create_window(uint16_t width, uint16_t height, const char* title)
{
    struct window* window = malloc(sizeof(struct window));
    
    window->h_instance = GetModuleHandleW(NULL);
    window->is_open = true;
    
    WNDCLASSW window_class;
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = window_procedure;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = window->h_instance;
    window_class.hIcon = NULL;
    window_class.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    
    RegisterClassW(&window_class);
    
    LPWSTR title_wide = malloc((strlen(title) + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, title, (int)(strlen(title) + 1), title_wide, (int)((strlen(title) + 1) * sizeof(wchar_t)));
    
    window->window = CreateWindowExW(0, WINDOW_CLASS_NAME, title_wide, WS_OVERLAPPEDWINDOW, 0, 0, width, height, NULL, NULL, window->h_instance, window);
    
    if (window->window == NULL)
    {
        fprintf(stderr, "[ERROR]: Failed to create Window '%s'\n", title);
        free(window);
        return NULL;
    }
    
    free(title_wide);
    
    return window;
}

const char** get_required_windowing_instance_extensions(uint32_t* extension_count)
{
    *extension_count = 2;
    char** extensions = malloc(*extension_count * sizeof(char*));
    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    
    return extensions;
}

VkSurfaceKHR create_surface_from_window(struct window* window, VkInstance instance, bool* status)
{
    VkWin32SurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.hinstance = window->h_instance;
    create_info.hwnd = window->window;
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[FATAL ERROR]: Failed to create the window surface. Vulkan error %d.", result);
        *status = false;
        return NULL;
    }
    
    *status = true;
    return surface;
}

bool is_window_open(struct window* window)
{
    return window->is_open;
}

void update_window(struct window* window)
{
    MSG message;
    PeekMessageW(&message, window->window, 0, 0, PM_REMOVE);
    TranslateMessage(&message);
    DispatchMessageW(&message);
}

void show_window(struct window* window)
{
    ShowWindow(window->window, SW_NORMAL);
}

void destroy_window(struct window* window)
{
    free(window);
}