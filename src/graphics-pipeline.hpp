#pragma once

namespace vulkan_scene
{
class Device;
class SwapChain;

class GraphicsPipeline
{
  public:
    GraphicsPipeline(const Device& device, const SwapChain& swap_chain,
                     std::string_view vertex_path,
                     std::string_view fragment_path);

    // Disable copying
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    VkRenderPass get_render_pass_raw_handle() const { return m_render_pass; }

    void cmd_bind(VkCommandBuffer command_buffer) const
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline);
    }

    ~GraphicsPipeline();

  private:
    // Called in the constructor and nowhere else.
    void create_layout();

    // Called in the constructor and nowhere else.
    void create_render_pass(const SwapChain& swap_chain);

    // Class members.
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    VkRenderPass m_render_pass;

    // Parent object.
    const Device& m_device;
};
} // namespace vulkan_scene