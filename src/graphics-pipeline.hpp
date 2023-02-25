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
                     std::string_view fragment_path,
                     uint16_t vertex_push_constant_range_size = 0,
                     uint16_t fragment_push_constant_range_size = 0,
                     bool enable_texture = false,
                     uint32_t max_set_count = 1);

    // Disable copying
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    
    // Allocates a descriptor set with the layout contained in the graphics pipeline.
    VkDescriptorSet allocate_descriptor_set() const;

    void cmd_bind(VkCommandBuffer command_buffer) const
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline);
    }

    inline void cmd_set_vertex_push_constants(VkCommandBuffer command_buffer,
                                              void* constants) const
    {
        vkCmdPushConstants(command_buffer, m_layout, VK_SHADER_STAGE_VERTEX_BIT,
                           0, m_vertex_push_constant_size, constants);
    }

    inline void cmd_set_fragment_push_constants(VkCommandBuffer command_buffer,
                                                const void* constants) const
    {
        vkCmdPushConstants(command_buffer, m_layout,
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                           m_vertex_push_constant_size,
                           m_fragment_push_constant_size, constants);
    }

    inline void cmd_bind_descriptor_set(VkCommandBuffer command_buffer,
                                        VkDescriptorSet descriptor_set) const
    {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_layout, 0, 1, &descriptor_set, 0, nullptr);
    }

    ~GraphicsPipeline();

  private:
    // Called in the constructor and nowhere else.
    void create_layout(uint16_t vertex_push_constant_range_size,
                       uint16_t fragment_push_constant_range_size,
                       bool enable_texture);
                       
    // Called in the constructor and nowhere else.
    void create_descriptor_pool(uint32_t max_set_count);

    // Class members.
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
    VkDescriptorSetLayout m_set_layout;
    
    VkDescriptorPool m_descriptor_pool;

    // Push constant sizes;
    uint16_t m_vertex_push_constant_size;
    uint16_t m_fragment_push_constant_size;
    
    // Whether there is a texture enabled or not
    bool m_enable_texture;

    // Parent object.
    const Device& m_device;
};

// A simple abstraction for a Vulkan description pool. It has support for RAII
// memory management and allocating descriptor sets.
} // namespace vulkan_scene