#ifndef INCLUDED_FRAMEBUFFER_MANAGER_H
#define INCLUDED_FRAMEBUFFER_MANAGER_H

#include <vulkan/vulkan.h>

struct framebuffer_manager
{
    const struct swap_chain* swap_chain;
    VkFramebuffer* framebuffers;
    uint32_t framebuffer_count;
};

/**
 * \brief Create a new framebuffer_manager structure instance. It allocates as 
 * much VkFramebuffers as there are VkImageViews in the swap_chain passed.
 * 
 * \param framebuffer_manager A pointer to the framebuffer_manager struct that 
 * the result of this function will be stored in.
 * \param swap_chain A read-only pointer to the swap_chain abstraction object
 * that this object will be created from.
 * 
 * \returns A boolean to indicate whether the function has succeeded or failed.
 */
bool create_new_framebuffer_manager(struct framebuffer_manager* framebuffer_manager, const struct swap_chain* swap_chain);

/**
 * \brief Destroys the framebuffer_manager by destroying the framebuffers on
 * the GPU and deallocating the array. The framebuffer_manager will no longer
 * be valid after this call, and attempts to use it as if it was valid will
 * result in undefined behaviour.
 * 
 * \param framebuffer_manager The `framebuffer_manager` instance that you would
 * like to destroy.
*/
void destroy_framebuffer_manager(struct framebuffer_manager* framebuffer_manager);

#endif