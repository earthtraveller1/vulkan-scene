#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

#include "vertex-buffer.h"

const VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION = {
    /* binding = */ 0,
    /* stride = */ sizeof(struct vertex),
    /* inputRate = */ VK_VERTEX_INPUT_RATE_VERTEX};

const VkVertexInputAttributeDescription
    VERTEX_ATTRIBUTE_DESCRIPTIONS[VERTEX_ATTRIBUTE_DESCRIPTION_COUNT] = {
        {/* location = */ 0,
         /* binding = */ 0,
         /* format = */ VK_FORMAT_R32G32B32_SFLOAT,
         /* offset = */ offsetof(struct vertex, position)}};

static uint32_t get_memory_type(uint32_t type_filter,
                                VkMemoryPropertyFlags properties,
                                VkPhysicalDevice physical_device, bool* found)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if (type_filter & (1 << i) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            *found = true;
            return i;
        }
    }

    *found = false;
    return 0;
}

static bool create_and_fill_staging_buffer(VkDevice device,
                                           VkPhysicalDevice physical_device,
                                           const struct vertex* data,
                                           size_t data_len, VkBuffer* buffer,
                                           VkDeviceMemory* memory)
{
    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.size = data_len * sizeof(struct vertex);
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = NULL;

    VkBuffer staging_buffer;
    VkResult result =
        vkCreateBuffer(device, &create_info, NULL, &staging_buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(
            stderr,
            "[ERROR]: Failed to create the staging buffer. Vulkan error %d.\n",
            result);
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, staging_buffer, &memory_requirements);

    bool found_memory_type = false;
    uint32_t memory_type =
        get_memory_type(memory_requirements.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        physical_device, &found_memory_type);
    if (!found_memory_type)
    {
        fprintf(stderr,
                "[ERROR]: Failed to find the memory type for the staging "
                "buffer. Vulkan error %d.\n",
                result);
        return false;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = data_len * sizeof(struct vertex);
    allocate_info.memoryTypeIndex = memory_type;

    VkDeviceMemory staging_buffer_memory;
    result =
        vkAllocateMemory(device, &allocate_info, NULL, &staging_buffer_memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to allocate memory for the staging buffer. "
                "Vulkan error %d.\n",
                result);
        return false;
    }

    vkBindBufferMemory(device, staging_buffer, staging_buffer_memory, 0);

    void* staging_buffer_ptr;
    vkMapMemory(device, staging_buffer_memory, 0,
                data_len * sizeof(struct vertex), 0, &staging_buffer_ptr);
    memcpy(staging_buffer_ptr, data, data_len * sizeof(struct vertex));
    vkUnmapMemory(device, staging_buffer_memory);

    /* Only if the buffer is filled do we return it to the caller. */
    *buffer = staging_buffer;
    *memory = staging_buffer_memory;

    return true;
}

static bool copy_buffer(const struct device* device, VkQueue queue,
                        VkBuffer source, VkBuffer destination,
                        VkDeviceSize buffer_size)
{
    VkCommandBuffer cmd_buffer;
    if (!create_new_command_buffer(&device->command_pool, &cmd_buffer))
    {
        fputs("[ERROR]: Failed to create the command buffer for copying "
              "buffers.\n",
              stderr);
        return false;
    }

    if (!begin_command_buffer(cmd_buffer, true))
    {
        fputs("[ERROR]: Failed to begin the command buffer for copying "
              "buffers.\n",
              stderr);
        return false;
    }

    VkBufferCopy copy_region;
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = buffer_size;

    vkCmdCopyBuffer(cmd_buffer, source, destination, 1, &copy_region);

    vkEndCommandBuffer(cmd_buffer);

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device->device, device->command_pool.command_pool, 1,
                         &cmd_buffer);

    return true;
}

bool create_vertex_buffer(struct vertex_buffer* self, const struct device* device,
                          const struct vertex* data, size_t data_len)
{
    self->device = device;

    const VkDeviceSize buffer_size = data_len * sizeof(struct vertex);

    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.size = buffer_size;
    create_info.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = NULL;

    VkResult result =
        vkCreateBuffer(device->device, &create_info, NULL, &self->buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to create a vertex buffer. Vulkan error %d.\n",
                result);
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->device, self->buffer,
                                  &memory_requirements);

    bool found_memory_type;
    uint32_t memory_type = get_memory_type(
        memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        device->physical_device, &found_memory_type);
    if (!found_memory_type)
    {
        fputs("[ERROR]: Was unable to locate an adequate memory type for the "
              "buffer.\n",
              stderr);
        return false;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type;

    result =
        vkAllocateMemory(device->device, &allocate_info, NULL, &self->memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to allocate memory. Vulkan error %d.\n",
                result);
        return false;
    }

    vkBindBufferMemory(device->device, self->buffer, self->memory, 0);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (!create_and_fill_staging_buffer(
            self->device->device, self->device->physical_device, data, data_len,
            &staging_buffer, &staging_buffer_memory))
    {
        fputs("[ERROR]: Failed to create a vertex buffer.\n", stderr);
        return false;
    }

    if (!copy_buffer(device, device->graphics_queue, staging_buffer,
                     self->buffer, buffer_size))
    {
        fputs("[ERROR]: Failed to copy the staging buffer onto the vertex "
              "buffer.\n",
              stderr);
        return false;
    }

    vkDestroyBuffer(self->device->device, staging_buffer, NULL);
    vkFreeMemory(self->device->device, staging_buffer_memory, NULL);

    return true;
}

void destroy_vertex_buffer(struct vertex_buffer* self)
{
    vkFreeMemory(self->device->device, self->memory, NULL);
    vkDestroyBuffer(self->device->device, self->buffer, NULL);
}
