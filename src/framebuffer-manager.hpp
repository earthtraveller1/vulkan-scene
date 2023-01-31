#pragma once

namespace vulkan_scene
{

class SwapChain;
class GraphicsPipeline;
class Device;

// The reason for this weird class is because of Vulkan's weird object
// dependency graph. I would like to include the framebuffers into a swap chain
// if I could but that would result in circular dependencies with the graphics
// pipeline as creating a VkFramebuffer requires a VkRenderPass object, but
// creating a VkRenderPass object requires the swap chain format.

// This is a very hacky solution, to be honest with you.

class FramebufferManager
{
  public:
    friend class SwapChain;

    ~FramebufferManager();

  private:
    FramebufferManager(const SwapChain& swap_chain,
                       const GraphicsPipeline& pipeline);

    std::vector<VkFramebuffer> m_framebuffers;

    const Device& m_device;
};

} // namespace vulkan_scene