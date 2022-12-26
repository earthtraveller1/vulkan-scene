#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "utils.h"

#include "buffers.h"

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

static bool create_and_fill_staging_buffer(const struct device* device,
                                           const void* data,
                                           VkDeviceSize buffer_size,
                                           VkBuffer* buffer,
                                           VkDeviceMemory* memory)
{
    if (!create_vulkan_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              device, buffer, memory))
    {
        fputs("[ERROR]: Failed to create the staging buffer.\n", stderr);
        return false;
    }

    void* buffer_ptr;
    vkMapMemory(device->device, *memory, 0, buffer_size, 0, &buffer_ptr);
#ifdef _MSC_VER
    memcpy_s(staging_buffer_ptr, buffer_size, data, buffer_size);
#else
    memcpy(buffer_ptr, data, buffer_size);
#endif
    vkUnmapMemory(device->device, *memory);

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

bool create_vertex_buffer(struct buffer* self, const struct device* device,
                          const struct vertex* data, size_t data_len)
{
    self->device = device;
    self->type = BUFFER_TYPE_VERTEX;

    const VkDeviceSize buffer_size = data_len * sizeof(struct vertex);

    if (!create_vulkan_buffer(buffer_size,
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device,
                              &self->buffer, &self->memory))
    {
        fputs("[ERROR]: Failed to create a Vulkan buffer.\n", stderr);
        return false;
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (!create_and_fill_staging_buffer(
            device, data, buffer_size, &staging_buffer, &staging_buffer_memory))
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

bool create_index_buffer(struct buffer* buffer, const struct device* device,
                         const uint32_t* data, size_t data_len)
{
    buffer->device = device;
    buffer->type = BUFFER_TYPE_INDEX;

    const VkDeviceSize buffer_size = data_len * sizeof(uint32_t);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (!create_and_fill_staging_buffer(
            device, data, buffer_size, &staging_buffer, &staging_buffer_memory))
    {
        fputs("[ERROR]: Failed to create a staging buffer for the index "
              "buffer.\n",
              stderr);
        return false;
    }

    if (!create_vulkan_buffer(buffer_size,
                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device,
                              &buffer->buffer, &buffer->memory))
    {
        fputs("[ERROR]: Failed to create an index buffer.\n", stderr);
        return false;
    }
    
    vkDestroyBuffer(device->device, staging_buffer, NULL);
    vkFreeMemory(device->device, staging_buffer_memory, NULL);
    
    return true;
}

void bind_buffer(const struct buffer* self, VkCommandBuffer cmd_buffer)
{
    if (self->type == BUFFER_TYPE_VERTEX)
    {
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &self->buffer, &offset);
    }
    else if (self->type == BUFFER_TYPE_INDEX)
    {
        vkCmdBindIndexBuffer(cmd_buffer, &self->buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void destroy_buffer(struct buffer* self)
{
    vkFreeMemory(self->device->device, self->memory, NULL);
    vkDestroyBuffer(self->device->device, self->buffer, NULL);
}
