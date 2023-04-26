#include <stdbool.h>
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

/* The actual handle to the window. */
static GLFWwindow* window = NULL;

bool create_window()
{
    glfwInit();

    /* I'll handle resizing later on. */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /* Tell GLFW to not create an OpenGL context, since we're not using OpenGL.
     */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(1280, 720, "Vulkan Scene", NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "[ERROR]: Failed to create a GLFW window.");
        return false;
    }

    return true;
}

/* This function returns the surface through the pointer. */
bool get_window_surface(VkInstance p_instance, VkSurfaceKHR* p_surface)
{
    VkResult result = glfwCreateWindowSurface(p_instance, window, NULL, p_surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create the Window surface.\n");
        return false;
    }

    return true;
}
    

bool is_window_open()
{
    return !glfwWindowShouldClose(window);
}

void get_framebuffer_size(int* width, int* height)
{
    glfwGetFramebufferSize(window, width, height);
}

void update_window()
{
    glfwPollEvents();
}

void destroy_window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
