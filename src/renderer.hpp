#pragma once

#include "device.hpp"
#include "buffers.hpp"
#include "swap-chain.hpp"
#include "graphics-pipeline.hpp"
#include "framebuffer-manager.hpp"
#include "render-pass.hpp"
#include "window.hpp"

namespace vulkan_scene
{

class Window;

// A basic rendering API abstraction
class Renderer
{
  public:
    Renderer(std::string_view app_name, bool enable_validation,
             const Window& window, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : m_device(app_name, enable_validation, window),
          m_swap_chain(m_device, window.get_width(), window.get_height()),
          m_vertex_buffer(m_device, vertices),
          m_index_buffer(m_device, indices),
          m_render_pass(m_swap_chain),
          m_pipeline(m_device, m_render_pass, "shaders/basic.vert.spv",
                     "shaders/basic.frag.spv"),
          m_framebuffers(m_swap_chain, m_render_pass),
          
          m_command_buffer(m_device.allocate_primary_cmd_buffer()),
          m_image_available_semaphore(m_device.create_semaphore()),
          m_render_done_semaphore(m_device.create_semaphore()),
          m_frame_fence(m_device.create_fence(true)),
          m_index_count(indices.size())
    {
    }

    void render();
    
    ~Renderer();
  private:
    Device m_device;
    SwapChain m_swap_chain;
    VertexBuffer m_vertex_buffer;
    IndexBuffer m_index_buffer;
    RenderPass m_render_pass;
    GraphicsPipeline m_pipeline;
    FramebufferManager m_framebuffers;

    // These currently do not have their own abstractions for now.
    VkCommandBuffer m_command_buffer;
    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_done_semaphore;
    VkFence m_frame_fence;
    
    // The number of indices to draw.
    const uint32_t m_index_count;
};

} // namespace vulkan_scene