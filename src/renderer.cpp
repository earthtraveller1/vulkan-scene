#include <limits>

#include "utils.hpp"

#include "renderer.hpp"

using vulkan_scene::Renderer;

void Renderer::begin()
{
    m_device.wait_for_fence(m_frame_fence);
    m_device.reset_fence(m_frame_fence);
    
    m_image_index = m_swap_chain.acquire_next_image(m_image_available_semaphore);
    
    vkResetCommandBuffer(m_command_buffer, 0);
    
    const VkCommandBufferBeginInfo cmd_buffer_begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    
    const auto result = vkBeginCommandBuffer(m_command_buffer, &cmd_buffer_begin_info);
    vulkan_scene_VK_CHECK(result, "begin recording the command buffer");
    
    m_render_pass.begin(m_command_buffer, m_framebuffers[m_image_index], 0.0f, 0.0f, 0.0f, 0.0f);
    
    m_pipeline.cmd_bind(m_command_buffer);
}

void Renderer::set_color_shift(float p_color_shift)
{
    const RendererPushConstants constants {
        .color_shift = p_color_shift
    };
    
    m_pipeline.cmd_set_fragment_push_constants(m_command_buffer, &constants);
}

void Renderer::draw()
{
    m_vertex_buffer.cmd_bind(m_command_buffer);
    m_index_buffer.cmd_bind(m_command_buffer);
    
    vkCmdDrawIndexed(m_command_buffer, m_index_count, 1, 0, 0, 0);
}

void Renderer::end()
{
    m_render_pass.end(m_command_buffer);
    
    auto result = vkEndCommandBuffer(m_command_buffer);
    vulkan_scene_VK_CHECK(result, "end recording the command buffer");
    
    const auto wait_stage = static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    
    const VkSubmitInfo submit_info{
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

    result = vkQueueSubmit(graphics_queue, 1, &submit_info, m_frame_fence);
    vulkan_scene_VK_CHECK(result, "submit the command buffer");

    m_swap_chain.present(m_render_done_semaphore, m_image_index);
}

Renderer::~Renderer()
{
    m_device.wait_idle();
    m_device.destroy_semaphore(m_image_available_semaphore);
    m_device.destroy_semaphore(m_render_done_semaphore);
    m_device.destroy_fence(m_frame_fence);
    m_device.free_command_buffer(m_command_buffer);
}