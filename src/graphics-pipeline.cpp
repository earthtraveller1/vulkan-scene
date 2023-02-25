#include <fstream>

#include "buffers.hpp"
#include "device.hpp"
#include "render-pass.hpp"
#include "swap-chain.hpp"
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
                                   const RenderPass& p_render_pass,
                                   std::string_view p_vertex_path,
                                   std::string_view p_fragment_path,
                                   uint16_t p_vertex_push_constant_size,
                                   uint16_t p_fragment_push_constant_size,
                                   bool p_enable_texture,
                                   uint32_t p_max_set_count)
    : m_vertex_push_constant_size(p_vertex_push_constant_size),
      m_fragment_push_constant_size(p_fragment_push_constant_size),
      m_device(p_device)
{
    create_layout(p_vertex_push_constant_size, p_fragment_push_constant_size,
                  p_enable_texture);
    create_descriptor_pool(p_max_set_count);

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
        .width =
            static_cast<float>(p_render_pass.m_swap_chain.get_extent().width),
        .height =
            static_cast<float>(p_render_pass.m_swap_chain.get_extent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    const VkRect2D scissor{
        .offset = {.x = 0, .y = 0},
        .extent = p_render_pass.m_swap_chain.get_extent()
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
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

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
        .renderPass = p_render_pass.m_render_pass,
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

VkDescriptorSet GraphicsPipeline::allocate_descriptor_set() const
{
    const VkDescriptorSetAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_set_layout
    };
    
    VkDescriptorSet descriptor_set;
    const auto result = vkAllocateDescriptorSets(m_device.get_raw_handle(), &alloc_info, &descriptor_set);
    vulkan_scene_VK_CHECK(result, "allocate a descriptor set from descriptor pool");
    
    return descriptor_set;
}

void GraphicsPipeline::create_layout(
    uint16_t p_vertex_push_constant_range_size,
    uint16_t p_fragment_push_constant_range_size, bool p_enable_texture)
{
    std::vector<VkPushConstantRange> push_constants;

    if (p_vertex_push_constant_range_size != 0)
    {
        push_constants.push_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = p_vertex_push_constant_range_size,
        });
    }

    if (p_fragment_push_constant_range_size != 0)
    {
        push_constants.push_back(
            VkPushConstantRange{.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                .offset = p_vertex_push_constant_range_size,
                                .size = p_fragment_push_constant_range_size});
    }
    
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    
    if (p_enable_texture)
    {
        bindings.push_back(VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        });
    }

    const VkDescriptorSetLayoutCreateInfo set_layout_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()};

    auto result = vkCreateDescriptorSetLayout(
        m_device.get_raw_handle(), &set_layout_create_info, nullptr, &m_set_layout);
    vulkan_scene_VK_CHECK(result, "create descriptor set layout");

    const auto create_info = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &m_set_layout,
        .pushConstantRangeCount = static_cast<uint32_t>(push_constants.size()),
        .pPushConstantRanges = push_constants.data()};

    result = vkCreatePipelineLayout(
        m_device.get_raw_handle(), &create_info, nullptr, &m_layout);
    vulkan_scene_VK_CHECK(result, "create a pipeline layout");
}

void GraphicsPipeline::create_descriptor_pool(uint32_t p_max_set_count)
{
    std::vector<VkDescriptorPoolSize> pool_sizes;
    
    if (m_enable_texture)
    {
        pool_sizes.push_back(VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1
        });
    }
    
    const VkDescriptorPoolCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = p_max_set_count,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data()
    };
    
    const auto result = vkCreateDescriptorPool(m_device.get_raw_handle(), &create_info, nullptr, &m_descriptor_pool);
    vulkan_scene_VK_CHECK(result, "create the descriptor pool for the pipeline");
}

GraphicsPipeline::~GraphicsPipeline()
{
    const auto raw_device = m_device.get_raw_handle();

    vkDestroyDescriptorPool(raw_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(raw_device, m_set_layout, nullptr);
    vkDestroyPipelineLayout(raw_device, m_layout, nullptr);
    vkDestroyPipeline(raw_device, m_pipeline, nullptr);
}

// DescriptorPool::DescriptorPool(const Device& p_device,
//                                std::span<const VkDescriptorPoolSize> p_sizes,
//                                uint32_t p_max_set_count)
//     : m_device(p_device)
// {
//     const VkDescriptorPoolCreateInfo create_info{
//         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .maxSets = p_max_set_count,
//         .poolSizeCount = static_cast<uint32_t>(p_sizes.size()),
//         .pPoolSizes = p_sizes.data()};

//     const auto result = vkCreateDescriptorPool(m_device.get_raw_handle(),
//                                                &create_info, nullptr, &m_pool);
//     vulkan_scene_VK_CHECK(result, "create a descriptor pool");
// }

// VkDescriptorSet
// DescriptorPool::allocate_set(VkDescriptorSetLayout p_layout) const
// {
//     const VkDescriptorSetAllocateInfo alloc_info{
//         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//         .pNext = nullptr,
//         .descriptorPool = m_pool,
//         .descriptorSetCount = 1,
//         .pSetLayouts = &p_layout};

//     VkDescriptorSet set;
//     const auto result =
//         vkAllocateDescriptorSets(m_device.get_raw_handle(), &alloc_info, &set);
//     vulkan_scene_VK_CHECK(result,
//                           "allocate a descriptor set from descriptor pool");
    
//     return set;
// }

// DescriptorPool::~DescriptorPool()
// {
//     vkDestroyDescriptorPool(m_device.get_raw_handle(), m_pool, nullptr);
// }