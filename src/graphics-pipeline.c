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

bool create_new_graphics_pipeline(struct graphics_pipeline* pipeline, struct device* device, const char* vertex_shader_path, const char* fragment_shader_path)
{
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
    
    vkDestroyShaderModule(device->device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device->device, fragment_shader_module, NULL);
    
    return true;
}

void destroy_graphics_pipeline(struct graphics_pipeline* pipeline)
{
    
}
