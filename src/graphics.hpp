#include <vulkan/vulkan.h>

namespace vulkan_scene {

auto create_graphics_pipeline(VkDevice p_device, VkRenderPass p_render_pass,
                              VkPipelineLayout p_layout,
                              VkShaderModule p_vertex_shader,
                              VkShaderModule p_fragment_shader) noexcept
    -> kirho::result_t<VkPipeline, VkResult>;

auto create_pipeline_layout(VkDevice p_device) noexcept
    -> kirho::result_t<VkPipelineLayout, VkResult>;

auto create_shader_module(VkDevice p_device,
                          std::string_view p_file_path) noexcept
    -> kirho::result_t<VkShaderModule, kirho::empty>;

} // namespace vulkan_scene
