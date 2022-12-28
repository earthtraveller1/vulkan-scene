#include <stdio.h>

#include "buffers.h"
#include "commands.h"
#include "device.h"
#include "framebuffer-manager.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"
#include "synchronization.h"
#include "utils.h"
#include "window.h"

#include "renderer.h"

bool create_new_renderer(struct renderer* self, struct window* window,
                         const char* app_name, bool enable_validation,
                         const char* vertex_shader_path,
                         const char* fragment_shader_path)
{
    self->window = window;
    self->vertex_buffer_valid = false;
    self->index_buffer_valid = false;

    if (!create_new_device(&self->device, app_name, enable_validation, window))
    {
        fputs("[ERROR]: Failed to create the device.\n", stderr);
        return false;
    }

    uint16_t window_width, window_height;
    get_window_size(window, &window_width, &window_height);

    if (!create_new_swap_chain(&self->swap_chain, &self->device, window_width,
                               window_height))
    {
        fputs("[ERROR]: Failed to create the swap chain.\n", stderr);
        return false;
    }

    if (!create_new_graphics_pipeline(&self->pipeline, &self->device,
                                      &self->swap_chain, vertex_shader_path,
                                      fragment_shader_path))
    {
        fputs("[ERROR]: Failed to create the graphics pipeline.\n", stderr);
        return false;
    }

    if (!create_new_framebuffer_manager(&self->framebuffers, &self->swap_chain,
                                        &self->pipeline))
    {
        fputs("[ERROR]: Failed to the framebuffers.\n", stderr);
        return false;
    }

    if (!create_vulkan_semaphore(&self->device,
                                 &self->semaphores.image_available))
    {
        fputs("[ERROR]: Failed to create the semaphore that is used to signal "
              "that the image is available.\n",
              stderr);
        return false;
    }

    if (!create_vulkan_semaphore(&self->device,
                                 &self->semaphores.render_finished))
    {
        fputs("[ERROR]: Failed to create the semaphore that is used to signal "
              "that the rendering has finished.\n",
              stderr);
        return false;
    }

    if (!create_vulkan_fence(&self->device, &self->frame_fence))
    {
        fputs("[ERROR]: Failed to create a Vulkan fence.\n", stderr);
        return false;
    }

    const struct command_pool* command_pool =
        get_command_pool_from_device(&self->device);
    if (!create_new_command_buffer(command_pool, &self->command_buffer))
    {
        fputs("[ERROR]: Failed to create a command buffer.\n", stderr);
        return false;
    }

    return true;
}

bool load_vertex_data_into_renderer(struct renderer* self, size_t vertex_count,
                                    const struct vertex* vertices)
{
    /* Only allow one call to this function. */
    if (self->vertex_buffer_valid)
    {
        fputs("[ERROR]: The renderer is already loaded with vertex data.\n",
              stderr);
        return false;
    }

    if (!create_vertex_buffer(&self->vertex_buffer, &self->device, vertices,
                              vertex_count))
    {
        fputs("[ERROR]: Failed to create a vertex buffer.\n", stderr);
        return false;
    }

    self->vertex_buffer_valid = true;
    return true;
}

bool load_indices_into_renderer(struct renderer* self, size_t index_count,
                                const uint32_t* indices)
{
    if (self->index_buffer_valid)
    {
        fputs("[ERRO]: The renderer is already loaded with indices.\n", stderr);
        return false;
    }

    if (!create_index_buffer(&self->index_buffer, &self->device, indices,
                             index_count))
    {
        fputs("[ERROR]: Failed to create an index buffer.\n", stderr);
        return false;
    }

    self->index_buffer_valid = true;
    return true;
}

bool begin_renderer(struct renderer* self, bool* recreate_swap_chain)
{
    *recreate_swap_chain = false;
    vkWaitForFences(self->device.device, 1, &self->frame_fence, VK_TRUE,
                    UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        self->device.device, self->swap_chain.swap_chain, UINT64_MAX,
        self->semaphores.image_available, VK_NULL_HANDLE, &self->image_index);
    if (result != VK_SUCCESS)
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            *recreate_swap_chain = true;
            return false;
        }
        else
        {
            fputs("[ERROR]: Failed to acquire an image from the swap chain.\n",
                  stderr);
            return false;
        }
    }

    vkResetFences(self->device.device, 1, &self->frame_fence);
    vkResetCommandBuffer(self->command_buffer, 0);

    if (!begin_command_buffer(self->command_buffer, false))
    {
        fputs("[ERROR]: Failed to begin a command buffer.\n", stderr);
        return false;
    }

    /* Begin a render pass and clear the screen. */

    begin_render_pass(0.0f, 0.0f, 0.0f, 1.0f, self->command_buffer,
                      &self->pipeline, &self->swap_chain, &self->framebuffers,
                      self->image_index);

    bind_graphics_pipeline(&self->pipeline, self->command_buffer);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)self->swap_chain.extent.width;
    viewport.height = (float)self->swap_chain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(self->command_buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = self->swap_chain.extent;

    vkCmdSetScissor(self->command_buffer, 0, 1, &scissor);

    return true;
}

bool recreate_renderer_swap_chain(struct renderer* self)
{
    uint16_t width, height;
    get_window_size(self->window, &width, &height);
    
    destroy_swap_chain(&self->swap_chain);
    if (!create_new_swap_chain(&self->swap_chain, &self->device, width, height))
    {
        fputs("[ERROR]: Failed to recreate the swap chain.\n", stderr);
        return false;
    }
    
    destroy_framebuffer_manager(&self->framebuffers);
    if (!create_new_framebuffer_manager(&self->framebuffers, &self->swap_chain, &self->pipeline))
    {
        fputs("[ERROR]: Failed to recreate the framebuffers.\n", stderr);
        return false;
    }
    
    return true;
}

void draw_triangle(struct renderer* self)
{
    bind_buffer(&self->vertex_buffer, self->command_buffer);

    vkCmdDraw(self->command_buffer, 3, 1, 0, 0);
}

void draw_polygon(struct renderer* self, uint32_t vertex_count,
                  float color_shift_amount)
{
    if (!self->vertex_buffer_valid || !self->index_buffer_valid)
    {
        fputs("[ERROR]: Either the vertex buffer doesn't exist or the index "
              "buffer doesn't exist.\n",
              stderr);
        return;
    }

    struct pipeline_push_constants push_constants;
    push_constants.color_shift_amount = color_shift_amount;

    set_graphics_pipeline_push_constants(&self->pipeline, self->command_buffer,
                                         &push_constants);

    bind_buffer(&self->vertex_buffer, self->command_buffer);
    bind_buffer(&self->index_buffer, self->command_buffer);

    vkCmdDrawIndexed(self->command_buffer, vertex_count, 1, 0, 0, 0);
}

bool end_renderer(struct renderer* self)
{
    vkCmdEndRenderPass(self->command_buffer);

    if (!end_command_buffer(self->command_buffer))
    {
        fputs("[ERROR]: Failed to end the command buffer.\n", stderr);
        return false;
    }

    const VkPipelineStageFlags wait_stages[1] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &self->semaphores.image_available;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &self->command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &self->semaphores.render_finished;

    PROFILE_INIT;

    VkResult result = vkQueueSubmit(self->device.graphics_queue, 1,
                                    &submit_info, self->frame_fence);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to submit the command buffer to the graphics "
                "queue. Vulkan error %d.\n",
                result);
        return false;
    }

    PROFILE_PRINT("Submitting the command buffer");

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &self->semaphores.render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &self->swap_chain.swap_chain;
    present_info.pImageIndices = &self->image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(self->device.present_queue, &present_info);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to present the swap chain. Vulkan error %d.\n",
                result);
        return false;
    }

    PROFILE_PRINT("Presenting to the swap chain");

    PROFILE_END;

    return true;
}

void destroy_renderer(struct renderer* self)
{
    /* Wait for the device to complete any operations. */
    device_wait_idle(&self->device);

    if (self->vertex_buffer_valid)
    {
        destroy_buffer(&self->vertex_buffer);
        self->vertex_buffer_valid = false; /* For correctness only. */
    }

    if (self->index_buffer_valid)
    {
        destroy_buffer(&self->index_buffer);
        self->index_buffer_valid = false; /* For correctness only. */
    }

    vkDestroySemaphore(self->device.device, self->semaphores.image_available,
                       NULL);
    vkDestroySemaphore(self->device.device, self->semaphores.render_finished,
                       NULL);
    vkDestroyFence(self->device.device, self->frame_fence, NULL);
    destroy_framebuffer_manager(&self->framebuffers);
    destroy_graphics_pipeline(&self->pipeline);
    destroy_swap_chain(&self->swap_chain);
    destroy_device(&self->device);
}
