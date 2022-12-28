#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <vulkan/vulkan.h>

#include "device.h"
#include "framebuffer-manager.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"

#include "utils.h"

size_t get_file_size(FILE* file)
{
    fseek(file, 0, SEEK_END);
    size_t result = ftell(file);
    fseek(file, 0, SEEK_SET);
    return result;
}

void begin_render_pass(float clear_color_r, float clear_color_g,
                       float clear_color_b, float clear_color_a,
                       VkCommandBuffer command_buffer,
                       const struct graphics_pipeline* pipeline,
                       const struct swap_chain* swap_chain,
                       const struct framebuffer_manager* framebuffers,
                       uint32_t image_index)
{
    VkClearValue clear_color;
    clear_color.color.float32[0] = clear_color_r;
    clear_color.color.float32[1] = clear_color_g;
    clear_color.color.float32[2] = clear_color_b;
    clear_color.color.float32[3] = clear_color_a;

    VkRenderPassBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_info.pNext = NULL;
    begin_info.renderPass = pipeline->render_pass;
    begin_info.framebuffer = framebuffers->framebuffers[image_index];
    begin_info.renderArea.extent = swap_chain->extent;
    begin_info.renderArea.offset.x = 0;
    begin_info.renderArea.offset.y = 0;
    begin_info.clearValueCount = 1;
    begin_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);
}

static bool get_memory_type(uint32_t type_filter,
                            VkMemoryPropertyFlags properties,
                            VkPhysicalDevice physical_device,
                            uint32_t* memory_type)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if (type_filter & (1 << i) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            *memory_type = i;
            return true;
        }
    }

    return false;
}

bool create_vulkan_buffer(VkDeviceSize buffer_size, VkBufferUsageFlagBits usage,
                          VkMemoryPropertyFlags memory_flags,
                          const struct device* device, VkBuffer* buffer,
                          VkDeviceMemory* memory)
{
    VkBufferCreateInfo buffer_create_info;
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = NULL;
    buffer_create_info.flags = 0;
    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;

    VkResult result =
        vkCreateBuffer(device->device, &buffer_create_info, NULL, buffer);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to create a Vulkan buffer. Vulkan error %d.\n",
                result);
        return false;
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->device, *buffer,
                                  &memory_requirements);

    uint32_t memory_type;
    if (!get_memory_type(memory_requirements.memoryTypeBits, memory_flags,
                         device->physical_device, &memory_type))
    {
        fprintf(
            stderr,
            "[ERROR]: Failed to obtain the memory type index of buffer %p.\n",
            (void*)*buffer);
        return false;
    }

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.memoryTypeIndex = memory_type;
    allocate_info.allocationSize = memory_requirements.size;

    result = vkAllocateMemory(device->device, &allocate_info, NULL, memory);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr,
                "[ERROR]: Failed to allocate device memory for Vulkan buffer "
                "%p. Vulkan error %d.\n",
                (void*)*buffer, result);
        return false;
    }

    vkBindBufferMemory(device->device, *buffer, *memory, 0);

    return true;
}
