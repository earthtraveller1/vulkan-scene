#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <vulkan/vulkan.h>

#include "graphics-pipeline.h"
#include "swap-chain.h"
#include "framebuffer-manager.h"

#include "utils.h"

size_t get_file_size(FILE* file)
{
    fseek(file, 0, SEEK_END);
    size_t result = ftell(file);
    fseek(file, 0, SEEK_SET);
    return result;
}

void begin_render_pass(float clear_color_r, float clear_color_g,
                       float clear_color_b, float clear_color_a,
                       VkCommandBuffer command_buffer,
                       const struct graphics_pipeline* pipeline,
                       const struct swap_chain* swap_chain,
                       const struct framebuffer_manager* framebuffers,
                       uint32_t image_index)
{
    VkClearValue clear_color;
    clear_color.color.float32[0] = clear_color_r;
    clear_color.color.float32[1] = clear_color_g;
    clear_color.color.float32[2] = clear_color_b;
    clear_color.color.float32[3] = clear_color_a;
    
    VkRenderPassBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.pNext = NULL;
    begin_info.renderPass = pipeline->render_pass;
    begin_info.framebuffer = framebuffers->framebuffers[image_index];
    begin_info.renderArea.extent = swap_chain->extent;
    begin_info.renderArea.offset.x = 0;
    begin_info.renderArea.offset.y = 0;
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}
