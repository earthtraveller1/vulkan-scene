#include <cstring>

#include "renderer.hpp"
#include "window.hpp"

using vulkan_scene::Renderer;
using vulkan_scene::Vertex;
using vulkan_scene::Window;

const uint16_t WIDTH = 1280;
const uint16_t HEIGHT = 720;

int main(int argc, char** argv)
{
    bool enable_validation = false;
    
    for (char** arg = argv; arg < argv + argc; arg++)
    {
        if (std::strcmp(*arg, "--enable-validation") == 0)
        {
            enable_validation = true;
        }
    }
    
    try
    {
        Window window("Vulkan Scene", WIDTH, HEIGHT);

        const std::vector<Vertex> vertices{Vertex{.position = {0.5f, -0.5f}},
                                           Vertex{.position = {0.5f, 0.5f}},
                                           Vertex{.position = {-0.5f, 0.5f}},
                                           Vertex{.position = {-0.5f, -0.5f}}};

        const std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};

        Renderer renderer("Vulkan Scene", enable_validation, window, vertices, indices);

        while (window.is_open())
        {
            renderer.render();

            window.update();
        }

        return 0;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "[FATAL ERROR]: " << e.what() << '\n';
    }
}