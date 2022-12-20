#include "device.h"
#include "swap-chain.h"
#include "commands.h"
#include "graphics-pipeline.h"
#include "framebuffer-manager.h"
#include "vertex-buffer.h"

#include "renderer.h"

bool draw(const struct rendering_data* data)
{
    uint32_t image_index;
    vkAcquireNextImageKHR(data->device->device, data->swap_chain->swap_chain, UINT64_MAX, data->image_available_semaphore, VK_NULL_HANDLE, &image_index);
    
    vkResetCommandBuffer(data->command_buffer, 0);
    
    /* Now, we start recording the command buffer. */
    
    if (!begin_command_buffer(data->command_buffer, false))
    {
        return false;
    }
    
    /* Begin a render pass. */
    
    VkClearValue clear_color;
    clear_color.color.float32[0] = 0.0f;
    clear_color.color.float32[1] = 0.0f;
    clear_color.color.float32[2] = 0.0f;
    clear_color.color.float32[3] = 0.0f;
    
    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = NULL;
    render_pass_begin_info.renderPass = data->pipeline->render_pass;
    render_pass_begin_info.framebuffer = data->framebuffers->framebuffers[image_index];
    render_pass_begin_info.renderArea.extent = data->swap_chain->extent;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(data->command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    
    vkCmdBindPipeline(data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipeline);
    
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = data->swap_chain->extent.width;
    viewport.height = data->swap_chain->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    vkCmdSetViewport(data->command_buffer, 0, 1, &viewport);
    
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = data->swap_chain->extent;
    
    vkCmdSetScissor(data->command_buffer, 0, 1, &scissor);
    
    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(data->command_buffer, 0, 1, &data->vertex_buffer->buffer, &offset);
    
    vkCmdDraw(data->command_buffer, 3, 1, 0, 0);
    
    vkCmdEndRenderPass(data->command_buffer);
}