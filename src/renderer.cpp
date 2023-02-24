#include <limits>

#include "utils.hpp"

#include "renderer.hpp"

using vulkan_scene::Renderer;

using namespace std::string_view_literals;

namespace
{
const std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{
    VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                         .descriptorCount = 1}
};

} // namespace

Renderer::Renderer(std::string_view app_name, bool enable_validation,
                   const Window& window, const std::vector<Vertex>& vertices,
                   const std::vector<uint32_t>& indices)
    : m_device(app_name, enable_validation, window),
      m_swap_chain(m_device, window.get_width(), window.get_height()),
      m_vertex_buffer(m_device, vertices), m_index_buffer(m_device, indices),
      m_render_pass(m_swap_chain),
      m_pipeline(m_device, m_render_pass, "shaders/basic.vert.spv",
                 "shaders/basic.frag.spv", 0, sizeof(RendererPushConstants), true),
      m_framebuffers(m_swap_chain, m_render_pass),
      m_texture(m_device, "images/bear.jpg"sv),

      m_command_buffer(m_device.allocate_primary_cmd_buffer()),
      m_image_available_semaphore(m_device.create_semaphore()),
      m_render_done_semaphore(m_device.create_semaphore()),
      m_frame_fence(m_device.create_fence(true)),

      m_descriptor_pool(m_device, descriptor_pool_sizes, 1),
      m_descriptor_set(
          m_descriptor_pool.allocate_set(m_pipeline.get_set_layout())),
      m_index_count(indices.size())
{
    const auto image_info = m_texture.get_image_info();
    const auto descriptor_write = m_texture.get_descriptor_write(m_descriptor_set, 0, &image_info);
    
    vkUpdateDescriptorSets(m_device.get_raw_handle(), 1, &descriptor_write, 0, nullptr);
}

void Renderer::begin()
{
    m_device.wait_for_fence(m_frame_fence);
    m_device.reset_fence(m_frame_fence);

    m_image_index =
        m_swap_chain.acquire_next_image(m_image_available_semaphore);

    vkResetCommandBuffer(m_command_buffer, 0);

    const VkCommandBufferBeginInfo cmd_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    const auto result =
        vkBeginCommandBuffer(m_command_buffer, &cmd_buffer_begin_info);
    vulkan_scene_VK_CHECK(result, "begin recording the command buffer");

    m_render_pass.begin(m_command_buffer, m_framebuffers[m_image_index], 0.0f,
                        0.0f, 0.0f, 0.0f);

    m_pipeline.cmd_bind(m_command_buffer);
}

void Renderer::set_color_shift(float p_color_shift)
{
    const RendererPushConstants constants{.color_shift = p_color_shift};

    m_pipeline.cmd_set_fragment_push_constants(m_command_buffer, &constants);
}

void Renderer::draw()
{
    m_vertex_buffer.cmd_bind(m_command_buffer);
    m_index_buffer.cmd_bind(m_command_buffer);
    
    m_pipeline.cmd_bind_descriptor_set(m_command_buffer, m_descriptor_set);

    vkCmdDrawIndexed(m_command_buffer, m_index_count, 1, 0, 0, 0);
}

void Renderer::end()
{
    m_render_pass.end(m_command_buffer);

    auto result = vkEndCommandBuffer(m_command_buffer);
    vulkan_scene_VK_CHECK(result, "end recording the command buffer");

    const auto wait_stage = static_cast<VkPipelineStageFlags>(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

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
    m_descriptor_pool.free_set(m_descriptor_set);
    m_device.wait_idle();
    m_device.destroy_semaphore(m_image_available_semaphore);
    m_device.destroy_semaphore(m_render_done_semaphore);
    m_device.destroy_fence(m_frame_fence);
    m_device.free_command_buffer(m_command_buffer);
}