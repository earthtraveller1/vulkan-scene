#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace vulkan_scene
{

enum class buffer_type_t
{
    VERTEX,
    INDEX
};

struct buffer_t
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    buffer_type_t type;
};

struct vertex_t
{
    glm::vec3 position;
};

auto create_render_pass(VkDevice p_device, VkFormat p_swapchain_format) noexcept
    -> kirho::result_t<VkRenderPass, VkResult>;

auto create_graphics_pipeline(
    VkDevice p_device,
    VkRenderPass p_render_pass,
    VkPipelineLayout p_layout,
    VkShaderModule p_vertex_shader,
    VkShaderModule p_fragment_shader
) noexcept -> kirho::result_t<VkPipeline, VkResult>;

auto create_pipeline_layout(VkDevice p_device) noexcept
    -> kirho::result_t<VkPipelineLayout, VkResult>;

auto create_shader_module(
    VkDevice p_device, std::string_view p_file_path
) noexcept -> kirho::result_t<VkShaderModule, kirho::empty_t>;

auto create_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkQueue graphics_queue,
    VkCommandPool command_pool,
    buffer_type_t type,
    const void* data,
    VkDeviceSize data_size
) noexcept -> kirho::result_t<buffer_t, VkResult>;

auto destroy_buffer(VkDevice device, const buffer_t& buffer) -> void;

} // namespace vulkan_scene
