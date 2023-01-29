#include <fstream>

#include "buffers.hpp"
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
                                   const SwapChain& p_swap_chain,
                                   std::string_view p_vertex_path,
                                   std::string_view p_fragment_path)
    : m_device(p_device)
{
    create_layout();
    create_render_pass(p_swap_chain);

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

    const auto shader_stages =
        std::array{vertex_shader_stage, fragment_shader_stage};

    const auto vertex_binding = Vertex::get_vertex_binding();
    const auto vertex_attributes = Vertex::get_vertex_attributes();

    const VkPipelineVertexInputStateCreateInfo vertex_input{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(vertex_attributes.size()),
        .pVertexAttributeDescriptions = vertex_attributes.data()};

    const VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    const VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(p_swap_chain.get_extent().width),
        .height = static_cast<float>(p_swap_chain.get_extent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    const VkRect2D scissor{
        .offset = {.x = 0, .y = 0},
          .extent = p_swap_chain.get_extent()
    };

    const VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    const VkPipelineRasterizationStateCreateInfo rasterization{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    const VkPipelineColorBlendAttachmentState color_attachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD};

    const VkPipelineColorBlendStateCreateInfo color_blending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    const VkGraphicsPipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &color_blending,
        .pDynamicState = nullptr,
        .layout = m_layout,
        .renderPass = m_render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0};

    const auto result =
        vkCreateGraphicsPipelines(p_device.get_raw_handle(), VK_NULL_HANDLE, 1,
                                  &create_info, nullptr, &m_pipeline);
    vulkan_scene_VK_CHECK(result, "create a graphics pipeline");

    // Note: These has to go to the VERY end of the function.
    vkDestroyShaderModule(p_device.get_raw_handle(), vertex_shader_module,
                          nullptr);
    vkDestroyShaderModule(p_device.get_raw_handle(), fragment_shader_module,
                          nullptr);
}

void GraphicsPipeline::create_layout()
{
    const auto create_info = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    const auto result = vkCreatePipelineLayout(
        m_device.get_raw_handle(), &create_info, nullptr, &m_layout);
    vulkan_scene_VK_CHECK(result, "create a pipeline layout");
}

void GraphicsPipeline::create_render_pass(const SwapChain& p_swap_chain)
{
    const auto color_attachment = VkAttachmentDescription{
        .flags = 0,
        .format = p_swap_chain.get_format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    const auto color_attachment_ref = VkAttachmentReference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const auto subpass = VkSubpassDescription{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr};

    const auto create_info = VkRenderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr};

    const auto result = vkCreateRenderPass(
        m_device.get_raw_handle(), &create_info, nullptr, &m_render_pass);
    vulkan_scene_VK_CHECK(result, "create a render pass");
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyRenderPass(m_device.get_raw_handle(), m_render_pass, nullptr);
    vkDestroyPipelineLayout(m_device.get_raw_handle(), m_layout, nullptr);
    vkDestroyPipeline(m_device.get_raw_handle(), m_pipeline, nullptr);
}