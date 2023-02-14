#include "swap-chain.hpp"

#include "render-pass.hpp"

using vulkan_scene::RenderPass;

RenderPass::RenderPass(const SwapChain& p_swap_chain)
    : m_device(p_swap_chain.get_parent_device()), m_swap_chain(p_swap_chain)
{
    const auto color_attachment = VkAttachmentDescription{
        .flags = 0,
        .format = p_swap_chain.get_format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    const auto color_attachment_ref = VkAttachmentReference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const auto dependency = VkSubpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    const auto subpass = VkSubpassDescription{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr};

    const auto create_info = VkRenderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    const auto result = vkCreateRenderPass(
        m_device.get_raw_handle(), &create_info, nullptr, &m_render_pass);
    vulkan_scene_VK_CHECK(result, "create a render pass");
}

void RenderPass::begin(VkCommandBuffer p_cmd_buffer,
                       VkFramebuffer p_framebuffer, float p_red, float p_green,
                       float p_blue, float p_alpha) const
{
    const VkClearValue clear_value{
        .color = {.float32 = {p_red, p_green, p_blue, p_alpha}}};

    const VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_render_pass,
        .framebuffer = p_framebuffer,
        .renderArea =
            {
                         .offset = {0, 0},
                         .extent = m_swap_chain.get_extent(),
                         },
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    vkCmdBeginRenderPass(p_cmd_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(VkCommandBuffer p_cmd_buffer) const
{
    vkCmdEndRenderPass(p_cmd_buffer);
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(m_device.get_raw_handle(), m_render_pass, nullptr);
}