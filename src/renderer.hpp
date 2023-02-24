#pragma once

#include "buffers.hpp"
#include "device.hpp"
#include "framebuffer-manager.hpp"
#include "graphics-pipeline.hpp"
#include "render-pass.hpp"
#include "swap-chain.hpp"
#include "window.hpp"

namespace vulkan_scene
{

class Window;

struct RendererPushConstants
{
    float color_shift;
};

// A basic rendering API abstraction
class Renderer
{
  public:
    Renderer(std::string_view app_name, bool enable_validation,
             const Window& window, const std::vector<Vertex>& vertices,
             const std::vector<uint32_t>& indices);

    void begin();
    
    void set_color_shift(float color_shift);

    void draw();
    
    void end();

    ~Renderer();

  private:
    Device m_device;
    SwapChain m_swap_chain;
    VertexBuffer m_vertex_buffer;
    IndexBuffer m_index_buffer;
    RenderPass m_render_pass;
    GraphicsPipeline m_pipeline;
    FramebufferManager m_framebuffers;
    Texture m_texture;

    // These currently do not have their own abstractions for now.
    VkCommandBuffer m_command_buffer;
    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_done_semaphore;
    VkFence m_frame_fence;
    
    // Descriptor related shit
    DescriptorPool m_descriptor_pool;
    VkDescriptorSet m_descriptor_set;

    // The number of indices to draw.
    const uint32_t m_index_count;
    
    // The image index currently used.
    uint32_t m_image_index;
};

} // namespace vulkan_scene