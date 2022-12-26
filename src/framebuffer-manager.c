#include <stdio.h>
#include <stdlib.h>


#include "graphics-pipeline.h"
#include "swap-chain.h"


#include "framebuffer-manager.h"

bool create_new_framebuffer_manager(struct framebuffer_manager* self,
                                    const struct swap_chain* swap_chain,
                                    const struct graphics_pipeline* pipeline)
{
    self->swap_chain = swap_chain;
    self->framebuffer_count = swap_chain->image_count;
    self->framebuffers =
        malloc(self->framebuffer_count * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < self->framebuffer_count; i++)
    {
        VkFramebufferCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.pNext = NULL;
        create_info.flags = 0;
        create_info.renderPass = pipeline->render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = swap_chain->image_views + i;
        create_info.width = swap_chain->extent.width;
        create_info.height = swap_chain->extent.height;
        create_info.layers = 1;

        VkResult result =
            vkCreateFramebuffer(swap_chain->device->device, &create_info, NULL,
                                self->framebuffers + i);
        if (result != VK_SUCCESS)
        {
            fprintf(
                stderr,
                "[ERROR]: Failed to create a framebuffer. Vulkan error %d.\n",
                result);
            return false;
        }
    }

    return true;
}

void destroy_framebuffer_manager(struct framebuffer_manager* self)
{
    for (const VkFramebuffer* framebuffer = self->framebuffers;
         framebuffer < self->framebuffers + self->framebuffer_count;
         framebuffer++)
    {
        vkDestroyFramebuffer(self->swap_chain->device->device, *framebuffer,
                             NULL);
    }

    free(self->framebuffers);
}
