#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "graphics-pipeline.h"

static bool load_shader_module(const char* filename, VkDevice device, VkShaderModule* shader_module)
{
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "[ERROR]: Failed to open %s.\n", filename);
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    uint8_t* file_contents = malloc(file_size);
    fread(file_contents, 1, file_size, file);
    
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.codeSize = file_size;
    create_info.pCode = (uint32_t*)file_contents;
    
    VkResult result = vkCreateShaderModule(device, &create_info, NULL, shader_module);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a shader module. Vulkan error %d.\n", result);
        return false;
    }
    
    free(file_contents);
    
    return true;
}

static bool create_pipeline_layout(struct graphics_pipeline* pipeline)
{
    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.setLayoutCount = 0;
    create_info.pushConstantRangeCount = 0;
    
    VkResult result = vkCreatePipelineLayout(pipeline->device->device, &create_info, NULL, &pipeline->layout);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create the pipeline layout.\n");
        return false;
    }
    
    return true;
}

bool create_new_graphics_pipeline(struct graphics_pipeline* pipeline, struct device* device, const char* vertex_shader_path, const char* fragment_shader_path)
{
    pipeline->device = device;
    
    VkShaderModule vertex_shader_module;
    if (!load_shader_module(vertex_shader_path, device->device, &vertex_shader_module))
    {
        fprintf(stderr, "[ERROR]: Failed to load and create the vertex shader module.\n");
        return false;
    }
    
    VkShaderModule fragment_shader_module;
    if (!load_shader_module(fragment_shader_path, device->device, &fragment_shader_module))
    {
        fputs("[ERROR]: Failed to create and load the framgnet shader module.\n", stderr);
        return false;
    }
    
    
    if (!create_pipeline_layout(pipeline))
    {
        return false;
    }
    
    VkPipelineShaderStageCreateInfo shader_stages[2];
    
    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].pNext = NULL;
    shader_stages[0].flags = 0;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vertex_shader_module;
    shader_stages[0].pName = "main";
    shader_stages[0].pSpecializationInfo = NULL;
    
    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].pNext = NULL;
    shader_stages[1].flags = 0;
    shader_stages[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[1].module = fragment_shader_module;
    shader_stages[1].pName = "main";
    shader_stages[1].pSpecializationInfo = NULL;
    
    const VkDynamicState dynamic_states[2] = {
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT
    };
    
    VkPipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.pNext = NULL;
    dynamic_state.flags = 0;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;
    
    /* TODO: Actually fill this out. */
    VkPipelineVertexInputStateCreateInfo vertex_input;
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = NULL;
    vertex_input.flags = 0;
    vertex_input.vertexBindingDescriptionCount = 0;
    vertex_input.pVertexBindingDescriptions = NULL;
    vertex_input.vertexAttributeDescriptionCount = 0;
    vertex_input.pVertexAttributeDescriptions = NULL;
    
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.pNext = NULL;
    input_assembly.flags = 0;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewport_state;
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = NULL;
    viewport_state.flags = 0;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = NULL;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = NULL;
    
    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = NULL;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.lineWidth = 1.0f;
    
    /* We might enable multisampling some time in the future, but not right now
    .*/
    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = NULL;
    multisampling.flags = 0;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 0.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    
    /* We might enable blending in the future, but for now, no. */
    VkPipelineColorBlendAttachmentState color_blending_attachment;
    color_blending_attachment.blendEnable = VK_FALSE;
    color_blending_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                               VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT |
                                               VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo color_blending;
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.pNext = NULL;
    color_blending.flags = 0;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_CLEAR;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blending_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;
    
    vkDestroyShaderModule(device->device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device->device, fragment_shader_module, NULL);
    
    return true;
}

void destroy_graphics_pipeline(struct graphics_pipeline* pipeline)
{
    vkDestroyPipelineLayout(pipeline->device->device, pipeline->layout, NULL);
}
