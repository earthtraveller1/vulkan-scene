#ifndef INCLUDED_SYNCHRONIZATION_H
#define INCLUDED_SYNCHRONIZATION_H

#include <vulkan/vulkan.h>

/**
 * \file A basic set of tools for managing GPU-side synchronization. Nothing
 * too complicated, for now.
 */

/**
 * \brief Creates a Vulkan semaphore. Needs to be destroyed with
 * `vkDestroySemaphore`.
 *
 * \param device The device object you would like to create this from.
 * \param semaphore The address that the created semaphore handle should be
 * stored at.
 *
 * \returns A boolean that indicates whether the procedure has succeeded.
 */
bool create_vulkan_semaphore(const struct device* device,
                             VkSemaphore* semaphore);

/**
 * \brief Creates a Vulkan fence. Needs to be destroyed with
 * `vkDestroyFence`.
 *
 * \param device The device object that you would like to create it from.
 * \param fence The address that the created fence handle should be
 * stored at.
 *
 * \returns Whether the procedure has failed or not.
 */
bool create_vulkan_fence(const struct device* device, VkFence* fence);

#endif