#pragma once

#include "device.hpp"
#include "window.hpp"

namespace vulkan_scene
{

class Window;

// A basic rendering API abstraction
class Renderer
{
  public:
    Renderer(std::string_view app_name, bool enable_validation,
             const Window& window, const std::vector<Vertex>& vertices)
        : m_device(app_name, enable_validation, window),
          m_swap_chain(m_device, window.get_width(), window.get_height()),
          m_vertex_buffer(m_device, vertices),
          m_pipeline(m_device, m_swap_chain, "shaders/basic.frag.spv",
                     "shaders/basic.vert.spv"),
          m_framebuffers(m_swap_chain, m_pipeline),
          m_command_buffer(m_device.allocate_primary_cmd_buffer()),
          m_image_available_semaphore(m_device.create_semaphore()),
          m_render_done_semaphore(m_device.create_semaphore()),
          m_frame_fence(m_device.create_fence(true))
    {
    }

    void render();

  private:
    Device m_device;
    SwapChain m_swap_chain;
    VertexBuffer m_vertex_buffer;
    GraphicsPipeline m_pipeline;
    FramebufferManager m_framebuffers;

    // These currently do not have their own abstractions for now.
    VkCommandBuffer m_command_buffer;
    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_done_semaphore;
    VkFence m_frame_fence;
};

} // namespace vulkan_scene