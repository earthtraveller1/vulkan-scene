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
    
    uint32_t* file_contents = malloc(file_size * 4);
    
    for (uint32_t* h = file_contents; h < file_contents + file_size; h++)
    {
        *h = getc(file);
    }
    
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.codeSize = file_size * 4;
    create_info.pCode = file_contents;
    
    VkResult result = vkCreateShaderModule(device, &create_info, NULL, shader_module);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[ERROR]: Failed to create a shader module. Vulkan error %d.\n", result);
        return false;
    }
    
    return true;
}

void create_new_graphics_pipeline(struct graphics_pipeline* pipeline, struct device* device)
{
    
}

void destroy_graphics_pipeline(struct graphics_pipeline* pipeline)
{
    
}
