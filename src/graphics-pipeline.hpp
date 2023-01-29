#pragma once

namespace vulkan_scene
{
class Device;

class GraphicsPipeline
{
  public:
    ~GraphicsPipeline();

  private:
    // Only the Device class should be allowed to create a GraphicsPipeline
    GraphicsPipeline(const Device& device, std::string_view vertex_path,
                     std::string_view fragment_path, uint16_t width, uint16_t height);

    // Class members.
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    VkRenderPass m_render_pass;

    // Parent object.
    const Device& m_device;
};
} // namespace vulkan_scene