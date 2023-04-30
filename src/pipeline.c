#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "graphics.h"

/* Contains all the implementation details relating to the graphics pipeline abstraction */

/* Creates a shader module by loading the SPIR-V from disk */
static bool create_shader_module_from_file(VkDevice p_device, const char* p_path, VkShaderModule* p_module)
{
    FILE* file = fopen(p_path, "r");

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

bool create_render_pass(VkDevice p_device, VkFormat p_swap_chain_format, VkRenderPass* p_render_pass)
{
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = p_swap_chain_format;
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
    create_info.dependencyCount = 0;
    create_info.pDependencies = NULL;

    VkResult result = vkCreateRenderPass(p_device, &create_info, NULL, p_render_pass);

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "\033[91m[ERROR]: Failed to create a render pass. Vulkan error %d.\033[0m\n", result);
        return false;
    }

    return true;
}

bool create_graphics_pipeline(VkDevice p_device, const char* p_vertex_path, const char* p_fragment_path, struct graphics_pipeline* p_pipeline)
{
    VkShaderModule vertex_module;
    if (!create_shader_module_from_file(p_device, p_vertex_path, &vertex_module))
        return false;

    VkShaderModule fragment_module;
    if (!create_shader_module_from_file(p_device, p_fragment_path, &fragment_module))
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

    VkPipelineShaderStageCreateInfo shader_stages[2] = { vertex_stage, fragment_stage };

    /* Note: These must go at the very end of the function, just before the return. */
    vkDestroyShaderModule(p_device, vertex_module, NULL);
    vkDestroyShaderModule(p_device, fragment_module, NULL);

    return true;
}
