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

VkSurfaceKHR get_window_surface(VkInstance p_instance)
{
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(p_instance, window, NULL, &surface);
    return surface;
}
    

bool is_window_open()
{
    return !glfwWindowShouldClose(window);
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
