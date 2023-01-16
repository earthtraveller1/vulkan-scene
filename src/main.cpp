#include "device.hpp"
#include "window.hpp"

using vulkan_scene::Device;
using vulkan_scene::Window;

int main()
{
    try
    {
        Window window("Vulkan Scene", 1280, 720);

        const Device device("Vulkan Scene", false, window);

        while (window.is_open())
        {
            window.update();
        }

        return 0;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "[FATAL ERROR]: " << e.what() << '\n';
    }
}