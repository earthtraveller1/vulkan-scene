#include <Windows.h>

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

const LPCTSTR WINDOW_CLASS_NAME = L"Vulkan Scene Window Class";

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
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = COLOR_BACKGROUND;
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    
    RegisterClassW(&window_class);
    
    window->window = CreateWindowExW(WS_OVERLAPPEDWINDOW, WINDOW_CLASS_NAME, title, 0, 0, 0, width, height, NULL, NULL, window->h_instance, window);
    
    if (window->window == NULL)
    {
        fprintf(stderr, "[ERROR]: Failed to create Window '%s'\n", title);
        free(window);
        return NULL;
    }
    
    return window;
}

void show_window(struct window* window)
{
    ShowWindow(window->window, SW_NORMAL);
}