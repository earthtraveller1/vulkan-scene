#ifndef INCLUDED_COMMANDS_H
#define INCLUDED_COMMANDS_H

#include <vulkan/vulkan.h>

#include "device.h"

/* A basic list of structures for working with command-related stuff in Vulkan,
such as command pools and buffers. */

/* A basic abstraction around Vulkan command pools. */
struct command_pool
{
    const struct device* device;
    VkCommandPool command_pool;
};

/* Create a new command pool. */
bool create_new_command_pool(const struct device* device, struct command_pool* command_pool);

/* Create a new command buffer. */
bool create_new_command_buffer(const struct command_pool* command_pool, VkCommandBuffer* command_buffer);

/* Begin recording a command buffer. */
bool begin_command_buffer(VkCommandBuffer command_buffer, bool one_time_use);

/* Stop recording a command buffer. */
bool end_command_buffer(VkCommandBuffer command_buffer);

/* Destroys the command pool. */
bool destroy_command_pool(struct command_pool* command_pool);

#endif