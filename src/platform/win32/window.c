#include <Windows.h>

#include <stdlib.h>
#include <stdio.h>

#include "../../window.h"

/* TODO: Implement the actual functions. */

struct window
{
    HINSTANCE h_instance; /* May be required for some things, but not completely sure. */
    HWND window;
};

const LPCTSTR WINDOW_CLASS_NAME = L"Vulkan Scene Window Class";

struct window* create_window(uint16_t width, uint16_t height, const char* title)
{
    struct window* window = malloc(sizeof(struct window));
    
    window->h_instance = GetModuleHandleW(NULL);
    
    WNDCLASSW window_class;
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = window->h_instance;
    window_class.hIcon = NULL;
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = COLOR_BACKGROUND;
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    
    RegisterClassW(&window_class);
    
    window->window = CreateWindowExW(WS_OVERLAPPEDWINDOW, WINDOW_CLASS_NAME, title, 0, 0, 0, width, height, NULL, NULL, window->h_instance, NULL);
    
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