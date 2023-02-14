#pragma once

namespace vulkan_scene
{
class Device;
class RenderPass;

class GraphicsPipeline
{
  public:
    GraphicsPipeline(const Device& device, const RenderPass& render_pass,
                     std::string_view vertex_path,
                     std::string_view fragment_path);

    // Disable copying
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    void cmd_bind(VkCommandBuffer command_buffer) const
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline);
    }

    ~GraphicsPipeline();

  private:
    // Called in the constructor and nowhere else.
    void create_layout();

    // Class members.
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;

    // Parent object.
    const Device& m_device;
};
} // namespace vulkan_scene