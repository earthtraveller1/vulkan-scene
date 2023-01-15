#include <iostream>

#include "device.hpp"
#include "window.hpp"

using vulkan_scene::Device;
using vulkan_scene::Window;

int main()
{
    Window window("Vulkan Scene", 1280, 720);

    const Device device("Vulkan Scene", false, window);

    while (window.is_open())
    {
        window.update();
    }

    return 0;
}