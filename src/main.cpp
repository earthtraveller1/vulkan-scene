#include "renderer.hpp"
#include "window.hpp"

using vulkan_scene::Renderer;
using vulkan_scene::Window;
using vulkan_scene::Vertex;

const uint16_t WIDTH = 1280;
const uint16_t HEIGHT = 720;

int main()
{
    try
    {
        Window window("Vulkan Scene", WIDTH, HEIGHT);
        
        const std::vector<Vertex> vertices {
            Vertex { .position = {  0.0f, -0.5f } },
            Vertex { .position = {  0.5f,  0.5f } },
            Vertex { .position = { -0.5f,  0.5f } }
        };
        
        Renderer renderer("Vulkan Scene", true, window, vertices);
        
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