#include "device.hpp"
#include "window.hpp"

using vulkan_scene::Device;
using vulkan_scene::Window;
using vulkan_scene::Vertex;

const uint16_t WIDTH = 1280;
const uint16_t HEIGHT = 720;

int main()
{
    try
    {
        Window window("Vulkan Scene", WIDTH, HEIGHT);

        const Device device("Vulkan Scene", true, window);
        
        const auto swap_chain = device.create_swap_chain(WIDTH, HEIGHT);
        
        // TODO: Fill out with actual vertex data.
        std::vector<Vertex> vertices {
            { { 0.0f, 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f } },
            { { 0.0f, 0.0f, 0.0f } }
        };
        
        const auto vertex_buffer = device.create_vertex_buffer(vertices);

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