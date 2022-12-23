#include <stdio.h>

#include "commands.h"
#include "device.h"
#include "framebuffer-manager.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"
#include "vertex-buffer.h"

#include "renderer.h"

/* bool draw(const struct rendering_data* data)
{
    vkWaitForFences(data->device->device, 1, &data->in_flight_fence, VK_TRUE,
                    UINT64_MAX);
    vkResetFences(data->device->device, 1, &data->in_flight_fence);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        data->device->device, data->swap_chain->swap_chain, UINT64_MAX,
        data->image_available_semaphore, VK_NULL_HANDLE, &image_index);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to retrieve an image from the swap chain. "
                "Vulkan error %d.\n",
                result);
        return false;
    }

    vkResetCommandBuffer(data->command_buffer, 0);

    /* Now, we start recording the command buffer. /

    if (!begin_command_buffer(data->command_buffer, false))
    {
        return false;
    }

    /* Begin a render pass. /

    VkClearValue clear_color;
    clear_color.color.float32[0] = 0.0f;
    clear_color.color.float32[1] = 0.0f;
    clear_color.color.float32[2] = 0.0f;
    clear_color.color.float32[3] = 0.0f;

    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = NULL;
    render_pass_begin_info.renderPass = data->pipeline->render_pass;
    render_pass_begin_info.framebuffer =
        data->framebuffers->framebuffers[image_index];
    render_pass_begin_info.renderArea.extent = data->swap_chain->extent;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(data->command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      data->pipeline->pipeline);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)data->swap_chain->extent.width;
    viewport.height = (float)data->swap_chain->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(data->command_buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = data->swap_chain->extent;

    vkCmdSetScissor(data->command_buffer, 0, 1, &scissor);

    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(data->command_buffer, 0, 1,
                           &data->vertex_buffer->buffer, &offset);

    vkCmdDraw(data->command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(data->command_buffer);

    if (!end_command_buffer(data->command_buffer))
    {
        return false;
    }

    const VkPipelineStageFlags wait_stages[1] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &data->image_available_semaphore;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &data->command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &data->render_finished_semaphore;

    result = vkQueueSubmit(data->device->graphics_queue, 1, &submit_info,
                           data->in_flight_fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to submit the command buffer to the queue. "
                "Vulkan error %d.\n",
                result);
        return false;
    }

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &data->render_finished_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &data->swap_chain->swap_chain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(data->device->present_queue, &present_info);
    if (result != VK_SUCCESS)
    {
        fprintf(
            stderr,
            "[ERROR]: Failed to present to the swap chain! Vulkan error %d.\n",
            result);
        return false;
    }

    return true;
} */

bool create_new_renderer(struct renderer* self, struct window* window,
                         const char* app_name, bool enable_validation,
                         const char* vertex_shader_path,
                         const char* fragment_shader_path)
{
    bool status = true;
    create_new_device(&self->device, app_name, enable_validation, window, &status);
    if (!status)
    {
        fputs("[ERROR]: Failed to create the device.\n", stderr);
        return false;
    }
    
    uint16_t window_width, window_height;
    get_window_size(window, &window_width, &window_height);
    
    if (!create_new_swap_chain(&self->swap_chain, &self->device, window_width, window_height))
    {
        fputs("[ERROR]: Failed to create the swap chain.\n", stderr);
        return false;
    }
}

void destroy_renderer(struct renderer* self)
{
    destroy_swap_chain(&self->swap_chain);
    destroy_device(&self->device);
}
