#include "utils.hpp"

#include "renderer.hpp"

using vulkan_scene::Renderer;

void Renderer::render()
{
    const auto device_raw = m_device.get_raw_handle();

    vkWaitForFences(device_raw, 1, &m_frame_fence, VK_TRUE,
                    (std::numeric_limits<uint64_t>::max)());
    vkResetFences(device_raw, 1, &m_frame_fence);

    const uint32_t image_index =
        m_swap_chain.acquire_next_image(m_image_available_semaphore);

    // This is part where we record into the command buffer.
    {
        vkResetCommandBuffer(m_command_buffer, 0);

        const VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr};

        auto result = vkBeginCommandBuffer(m_command_buffer, &begin_info);
        vulkan_scene_VK_CHECK(result, "begin a command buffer");

        const VkClearValue clear_value{
            .color = {.float32 = {1.0f, 1.0f, 0.0f, 1.0f}}};

        const VkRenderPassBeginInfo render_pass_begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = m_pipeline.get_render_pass_raw_handle(),
            .framebuffer = m_framebuffers[image_index],
            .renderArea = {.extent = m_swap_chain.get_extent(),
                           .offset = {0, 0}},
            .clearValueCount = 1,
            .pClearValues = &clear_value
        };
        
        vkCmdBeginRenderPass(m_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        
        m_pipeline.cmd_bind(m_command_buffer);
        m_vertex_buffer.cmd_bind(m_command_buffer);
        
        vkCmdDraw(m_command_buffer, 3, 1, 0, 0);
        
        vkCmdEndRenderPass(m_command_buffer);
        
        result = vkEndCommandBuffer(m_command_buffer);
        vulkan_scene_VK_CHECK(result, "stop recording the command buffer");
    }
    
    const auto wait_stage = static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    
    const VkSubmitInfo submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_image_available_semaphore,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_render_done_semaphore,
    };
    
    const auto graphics_queue = m_device.get_graphics_queue();
    
    auto result = vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vulkan_scene_VK_CHECK(result, "submit the command buffer");
    
    m_swap_chain.present(m_render_done_semaphore, image_index);
}