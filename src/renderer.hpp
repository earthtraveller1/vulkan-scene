#pragma once

#include "device.hpp"

namespace vulkan_scene
{
    
class Window;

// A basic rendering API abstraction
class Renderer
{
public:
    Renderer(std::string_view app_name, bool enable_validation, const Window& window, const std::vector<Vertex>& vertices);
    
    void render();
    
private:
    Device m_device;
    SwapChain m_swap_chain;
    VertexBuffer m_vertex_buffer;
    GraphicsPipeline m_pipeline;
    FramebufferManager m_framebuffers;
};

}