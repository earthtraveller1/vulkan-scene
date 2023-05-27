#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "device.h"

#include "graphics.h"
#include "swapchain.h"

/* Contains all the implementation details relating to the graphics pipeline abstraction */

const static VkVertexInputBindingDescription vertex_binding_description = {
    /* binding = */ 0,
    /* stride = */ sizeof(struct vertex),
    /* inputRate = */ VK_VERTEX_INPUT_RATE_VERTEX};

#define VERTEX_ATTRIBUTE_COUNT 1

const static VkVertexInputAttributeDescription vertex_attribute_descriptions[VERTEX_ATTRIBUTE_COUNT] = {
    /* VkVertexInputAttributeDescription */ {/* binding = */ 0,
                                             /* location = */ 0,
                                             /* format = */ VK_FORMAT_R32G32_SFLOAT,
                                             /* offset = */ offsetof(struct vertex, position)}};

/* Creates a shader module by loading the SPIR-V from disk */
static bool create_shader_module_from_file(VkDevice p_device, const char* p_path, VkShaderModule* p_module)
{
    FILE* file = fopen(p_path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to open file %s. Maybe it doesn't exist?\033[0m\n", p_path);
        return false;
    }

    /* Obtain the size of the file by seeking to the end and obtaining the position. */
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET); /* Remember to go back to the beginning */

    char* file_contents = malloc(sizeof(char) * file_size);
    fread(file_contents, sizeof(char), file_size, file);

    fclose(file);

    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.codeSize = file_size;
    create_info.pCode = (uint32_t*)file_contents;

    VkResult result = vkCreateShaderModule(p_device, &create_info, NULL, p_module);

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create shader module %s. Vulkan error %d\033[0m\n", p_path, result);
        return false;
    }

    free(file_contents);

    return true;
}

/* Creates a pipeline layout. Duh. Well, of course, this is going to change in the future as I add more uniforms and stuff to it. */
static bool create_pipeline_layout(VkDevice p_device, VkPipelineLayout* p_layout)
{
    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = NULL;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = NULL;

    VkResult result = vkCreatePipelineLayout(p_device, &create_info, NULL, p_layout);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create the pipeline layout. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

bool create_render_pass(VkRenderPass* p_render_pass)
{
    VkFormat swap_chain_format = get_swap_chain_format();

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = swap_chain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref;
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency subpass_dependency;
    subpass_dependency.dependencyFlags = 0;
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &subpass_dependency;

    VkDevice device = get_global_logical_device();

    VkResult result = vkCreateRenderPass(device, &create_info, NULL, p_render_pass);

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create a render pass. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

bool create_graphics_pipeline(const char* p_vertex_path, const char* p_fragment_path, VkRenderPass p_render_pass,
                              struct graphics_pipeline* p_pipeline)
{
    VkDevice device = get_global_logical_device();

    VkShaderModule vertex_module;
    if (!create_shader_module_from_file(device, p_vertex_path, &vertex_module))
        return false;

    VkShaderModule fragment_module;
    if (!create_shader_module_from_file(device, p_fragment_path, &fragment_module))
        return false;

    VkPipelineShaderStageCreateInfo vertex_stage;
    vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage.pNext = NULL;
    vertex_stage.flags = 0;
    vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage.module = vertex_module;
    vertex_stage.pName = "main";
    vertex_stage.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo fragment_stage;
    fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage.pNext = NULL;
    fragment_stage.flags = 0;
    fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage.module = fragment_module;
    fragment_stage.pName = "main";
    fragment_stage.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shader_stages[2] = {vertex_stage, fragment_stage};

    VkPipelineVertexInputStateCreateInfo vertex_input_state;
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.pNext = NULL;
    vertex_input_state.flags = 0;
    vertex_input_state.vertexBindingDescriptionCount = 1;
    vertex_input_state.pVertexBindingDescriptions = &vertex_binding_description;
    vertex_input_state.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
    vertex_input_state.pVertexAttributeDescriptions = vertex_attribute_descriptions;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
    input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.pNext = NULL;
    input_assembly_state.flags = 0;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state;
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = NULL;
    viewport_state.flags = 0;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = NULL;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizer_stage;
    rasterizer_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_stage.pNext = NULL;
    rasterizer_stage.flags = 0;
    rasterizer_stage.depthClampEnable = VK_FALSE;
    rasterizer_stage.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_stage.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_stage.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_stage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_stage.depthBiasEnable = VK_FALSE;
    rasterizer_stage.depthBiasConstantFactor = 0.0f;
    rasterizer_stage.depthBiasClamp = 0.0f;
    rasterizer_stage.depthBiasSlopeFactor = 0.0f;
    rasterizer_stage.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling_state;
    multisampling_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_state.pNext = NULL;
    multisampling_state.flags = 0;
    multisampling_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_state.sampleShadingEnable = VK_FALSE;
    multisampling_state.minSampleShading = 1.0f;
    multisampling_state.pSampleMask = NULL;
    multisampling_state.alphaToCoverageEnable = VK_FALSE;
    multisampling_state.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state;
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.pNext = NULL;
    color_blend_state.flags = 0;
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &color_blend_attachment_state; /* TODO. */
    color_blend_state.blendConstants[0] = 0.0f;
    color_blend_state.blendConstants[1] = 0.0f;
    color_blend_state.blendConstants[2] = 0.0f;
    color_blend_state.blendConstants[3] = 0.0f;

    const VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};

    VkPipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext = NULL;
    dynamic_state.flags = 0;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    if (!create_pipeline_layout(device, &p_pipeline->layout))
        return false;

    VkGraphicsPipelineCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.stageCount = 2;
    create_info.pStages = shader_stages;
    create_info.pVertexInputState = &vertex_input_state;
    create_info.pInputAssemblyState = &input_assembly_state;
    create_info.pTessellationState = NULL;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterizer_stage;
    create_info.pMultisampleState = &multisampling_state;
    create_info.pDepthStencilState = NULL;
    create_info.pColorBlendState = &color_blend_state;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = p_pipeline->layout;
    create_info.renderPass = p_render_pass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, NULL, &p_pipeline->pipeline);

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create a graphics pipeline. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    /* Note: These must go at the very end of the function, just before the return. */
    vkDestroyShaderModule(device, vertex_module, NULL);
    vkDestroyShaderModule(device, fragment_module, NULL);

    return true;
}

void destroy_render_pass(VkRenderPass p_render_pass) { vkDestroyRenderPass(get_global_logical_device(), p_render_pass, NULL); }

void destroy_graphics_pipeline(const struct graphics_pipeline* p_pipeline)
{
    VkDevice device = get_global_logical_device();

    vkDestroyPipelineLayout(device, p_pipeline->layout, NULL);
    vkDestroyPipeline(device, p_pipeline->pipeline, NULL);
}
