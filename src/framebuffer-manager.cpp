#include "device.hpp"
#include "graphics-pipeline.hpp"
#include "swap-chain.hpp"
#include "utils.hpp"

#include "framebuffer-manager.hpp"

using vulkan_scene::FramebufferManager;

FramebufferManager::FramebufferManager(const SwapChain& p_swap_chain,
                                       const GraphicsPipeline& p_pipeline)
    : m_framebuffers(p_swap_chain.m_image_views.size()),
      m_device(p_swap_chain.m_device)
{
    std::transform(
        p_swap_chain.m_image_views.begin(), p_swap_chain.m_image_views.end(),
        m_framebuffers.begin(),
        [&p_pipeline, &p_swap_chain](VkImageView p_image_view) -> VkFramebuffer
        {
            const VkFramebufferCreateInfo create_info{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = p_pipeline.get_render_pass_raw_handle(),
                .attachmentCount = 1,
                .pAttachments = &p_image_view,
                .width = p_swap_chain.m_extent.width,
                .height = p_swap_chain.m_extent.height,
                .layers = 1};

            VkFramebuffer framebuffer;
            const auto result =
                vkCreateFramebuffer(p_swap_chain.m_device.get_raw_handle(),
                                    &create_info, nullptr, &framebuffer);
            vulkan_scene_VK_CHECK(result, "create a swap chain framebuffer");

            return framebuffer;
        });
}

FramebufferManager::~FramebufferManager()
{
    std::for_each(m_framebuffers.begin(), m_framebuffers.end(),
                  [this](VkFramebuffer framebuffer) {
                      vkDestroyFramebuffer(m_device.get_raw_handle(),
                                           framebuffer, nullptr);
                  });
}