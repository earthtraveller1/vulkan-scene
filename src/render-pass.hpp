#pragma once

namespace vulkan_scene
{
class SwapChain;
class Device;

// An abstraction for the render pass, which is used to create other
// objects like the graphics pipeline and the swap chain.
class RenderPass
{
  public:
    // The graphics pipeline needs to access the raw handle.
    friend class GraphicsPipeline;

    RenderPass(const SwapChain& swap_chain);

    // Begins the render pass.
    void begin(VkCommandBuffer cmd_buffer, VkFramebuffer framebuffer, float red,
               float green, float blue, float alpha) const;

    // Ends the render pass.
    void end(VkCommandBuffer cmd_buffer) const;

    ~RenderPass();

  private:
    // The parent device handle.
    const Device& m_device;

    // The target swap chain.
    const SwapChain& m_swap_chain;

    // The raw handle.
    VkRenderPass m_render_pass;
};
} // namespace vulkan_scene