#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "graphics.h"
#include "swapchain.h"
#include "utils.h"
#include "window.h"

int main(int argc, const char* const* const argv)
{
    bool enable_validation = false;

    for (const char* const* arg = argv; arg < argv + argc; arg++)
    {
        if (strcmp(*arg, "--enable-validation") == 0)
            enable_validation = true;
    }

    if (!create_window())
        return EXIT_FAILURE;

    if (!create_device(enable_validation))
        return EXIT_FAILURE;

    if (!create_swap_chain())
        return EXIT_FAILURE;

    VkRenderPass render_pass;
    if (!create_render_pass(&render_pass))
        return EXIT_FAILURE;

    VkFramebuffer* swap_chain_framebuffers;
    uint32_t swap_chain_framebuffer_count;
    if (!create_swap_chain_framebuffers(render_pass, &swap_chain_framebuffers, &swap_chain_framebuffer_count))
        return EXIT_FAILURE;

    struct graphics_pipeline pipeline;
    if (!create_graphics_pipeline("shaders/basic.vert.spv", "shaders/basic.frag.spv", render_pass, 0, 0, &pipeline))
        return EXIT_FAILURE;

    struct vertex vertices[8] = {{{0.1f + 0.5f, -0.1f, 0.0f}},  {{0.1f + 0.5f, 0.1f, 0.0f}},  {{-0.1f + 0.5f, 0.1f, 0.0f}},
                                 {{-0.1f + 0.5f, -0.1f, 0.0f}}, {{0.1f - 0.5f, -0.1f, 0.0f}}, {{0.1f - 0.5f, 0.1f, 0.0f}},
                                 {{-0.1f - 0.5f, 0.1f, 0.0f}},  {{-0.1f - 0.5f, -0.1f, 0.0f}}};

    struct buffer vertex_buffer;
    if (!create_buffer(vertices, sizeof(vertices), BUFFER_TYPE_VERTEX, &vertex_buffer))
        return EXIT_FAILURE;

    uint16_t indices[12] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

    struct buffer index_buffer;
    if (!create_buffer(indices, sizeof(indices), BUFFER_TYPE_INDEX, &index_buffer))
        return EXIT_FAILURE;

    VkSemaphore image_available_semaphore;
    if (!create_semaphore(&image_available_semaphore))
        return EXIT_FAILURE;

    VkSemaphore render_done_semaphore;
    if (!create_semaphore(&render_done_semaphore))
        return EXIT_FAILURE;

    VkFence frame_fence;
    if (!create_fence(&frame_fence))
        return EXIT_FAILURE;

    VkCommandBuffer command_buffer;
    if (!allocate_command_buffer(&command_buffer))
        return EXIT_FAILURE;

    VkDevice device = get_global_logical_device();
    VkQueue graphics_queue = get_global_graphics_queue();
    VkQueue present_queue = get_global_present_queue();

    VkSwapchainKHR swap_chain = get_global_swap_chain();
    VkExtent2D swap_extent = get_swap_chain_extent();

    int exit_status = EXIT_SUCCESS;

    while (is_window_open())
    {
        vkWaitForFences(device, 1, &frame_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &frame_fence);

        uint32_t image_index;
        if (!swap_chain_acquire_next_image(&image_index, image_available_semaphore))
        {
            exit_status = EXIT_FAILURE;
            break;
        }

        vkResetCommandBuffer(command_buffer, 0);

        /* Start recording the command buffer. */

        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = NULL;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = NULL;

        VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
        HANDLE_VK_ERROR(result, "begin the command buffer for drawing", exit_status = EXIT_FAILURE; break);

        /* Begin the render pass. */

        VkClearValue clear_value;
        clear_value.color.float32[0] = 0.0f;
        clear_value.color.float32[1] = 0.0f;
        clear_value.color.float32[2] = 0.0f;
        clear_value.color.float32[3] = 0.0f;

        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.pNext = NULL;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = swap_chain_framebuffers[image_index];
        render_pass_begin_info.renderArea.extent = swap_extent;
        render_pass_begin_info.renderArea.offset.x = 0;
        render_pass_begin_info.renderArea.offset.y = 0;
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_value;

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        /* Bind the graphics pipeline. */

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

        /* Set the dynamic states. */

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swap_extent.width;
        viewport.height = (float)swap_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = swap_extent;

        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        /* Bind the vertex buffer. */

        const VkDeviceSize buffer_offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer.buffer, &buffer_offset);

        /* Bind the index buffer. */
        vkCmdBindIndexBuffer(command_buffer, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

        /* Issue the draw command. */

        /* vkCmdDraw(command_buffer, 3, 1, 0, 0); */

        vkCmdDrawIndexed(command_buffer, 12, 1, 0, 0, 0);

        /* Now, we can just end everything. */

        vkCmdEndRenderPass(command_buffer);

        result = vkEndCommandBuffer(command_buffer);
        HANDLE_VK_ERROR(result, "stop recording the command buffer for drawing", exit_status = EXIT_FAILURE; break);

        /* Submit the command buffer to the graphics queue. */

        const VkPipelineStageFlags wait_stages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_semaphore;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_done_semaphore;

        result = vkQueueSubmit(graphics_queue, 1, &submit_info, frame_fence);
        HANDLE_VK_ERROR(result, "submit the draw command buffer to the render queue", exit_status = EXIT_FAILURE; break);

        VkPresentInfoKHR present_info;
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = NULL;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_done_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain;
        present_info.pImageIndices = &image_index;
        present_info.pResults = NULL;

        result = vkQueuePresentKHR(present_queue, &present_info);
        HANDLE_VK_ERROR(result, "present to the screen with the present queue", exit_status = EXIT_FAILURE; break);

        update_window();
    }

    vkDeviceWaitIdle(device);

    vkFreeCommandBuffers(device, get_global_command_pool(), 1, &command_buffer);
    vkDestroyFence(device, frame_fence, NULL);
    vkDestroySemaphore(device, render_done_semaphore, NULL);
    vkDestroySemaphore(device, image_available_semaphore, NULL);

    destroy_buffer(&index_buffer);
    destroy_buffer(&vertex_buffer);
    destroy_graphics_pipeline(&pipeline);
    destroy_swap_chain_framebuffers(swap_chain_framebuffers, swap_chain_framebuffer_count);
    destroy_render_pass(render_pass);
    destroy_swapchain();
    destroy_window();
    destroy_device();

    return exit_status;
}
