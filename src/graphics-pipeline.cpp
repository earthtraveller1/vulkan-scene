#include <fstream>

#include "device.hpp"
#include "utils.hpp"

#include "graphics-pipeline.hpp"

using vulkan_scene::GraphicsPipeline;
using namespace std::string_literals;

namespace
{
VkShaderModule load_and_create_shader_module(std::string_view p_path,
                                             VkDevice p_device)
{
    std::ifstream file(p_path.data(), std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Failed to open "s + std::string(p_path) +
                                 '.');
    }

    const auto file_size = static_cast<std::size_t>(file.tellg());
    file.seekg(0);

    std::vector<char> file_contents(file_size);
    file.read(file_contents.data(), file_size);

    const VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = file_contents.size(),
        .pCode = reinterpret_cast<uint32_t*>(file_contents.data())};

    VkShaderModule shader_module;
    const auto result =
        vkCreateShaderModule(p_device, &create_info, nullptr, &shader_module);
    vulkan_scene_VK_CHECK(result, "create a shader module");

    return shader_module;
}
} // namespace

GraphicsPipeline::GraphicsPipeline(const Device& p_device,
                                   std::string_view p_vertex_path,
                                   std::string_view p_fragment_path)
    : m_device(p_device)
{
    const auto vertex_shader_module =
        load_and_create_shader_module(p_vertex_path, p_device.get_raw_handle());
    const auto fragment_shader_module = load_and_create_shader_module(
        p_fragment_path, p_device.get_raw_handle());

    const VkPipelineShaderStageCreateInfo vertex_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    const VkPipelineShaderStageCreateInfo fragment_shader_stage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    // Note: These has to go to the VERY end of the function.
    vkDestroyShaderModule(p_device.get_raw_handle(), vertex_shader_module,
                          nullptr);
    vkDestroyShaderModule(p_device.get_raw_handle(), fragment_shader_module,
                          nullptr);
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyRenderPass(m_device.get_raw_handle(), m_render_pass, nullptr);
    vkDestroyPipelineLayout(m_device.get_raw_handle(), m_layout, nullptr);
    vkDestroyPipeline(m_device.get_raw_handle(), m_pipeline, nullptr);
}