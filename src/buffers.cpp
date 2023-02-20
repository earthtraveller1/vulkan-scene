#include <cstring>

#include "stb_image.h"

#include "device.hpp"
#include "utils.hpp"

#include "buffers.hpp"

using namespace std::string_literals;
using vulkan_scene::Device;
using vulkan_scene::IndexBuffer;
using vulkan_scene::Texture;
using vulkan_scene::VertexBuffer;

namespace
{
enum class BufferType
{
    Vertex,
    Index,
    Uniform,
    Staging
};

std::uint32_t find_memory_type_index(VkPhysicalDevice p_device,
                                     std::uint32_t p_type_filter,
                                     VkMemoryPropertyFlags p_properties)
{
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(p_device, &properties);

    for (std::uint32_t i = 0; i < properties.memoryTypeCount; i++)
    {
        if ((p_type_filter & (1 << i)) &&
            (properties.memoryTypes[i].propertyFlags & p_properties) ==
                p_properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find the memory type index.");
}

template <BufferType buffer_type>
std::tuple<VkBuffer, VkDeviceMemory>
create_buffer(const vulkan_scene::Device& p_device, VkDeviceSize p_size)
{
    const VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_size,
        .usage = []() -> int
        {
            switch (buffer_type)
            {
            case BufferType::Vertex:
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case BufferType::Index:
                return VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            case BufferType::Staging:
                return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            default:
                return 0;
            }
        }(),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0};

    VkBuffer buffer;
    const auto result = vkCreateBuffer(p_device.get_raw_handle(), &create_info,
                                       nullptr, &buffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a buffer. Vulkan error "s +
                                 std::to_string(result) + '.');
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(p_device.get_raw_handle(), buffer,
                                  &memory_requirements);

    const auto memory_properties =
        buffer_type == BufferType::Staging
            ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    std::uint32_t memory_type_index = find_memory_type_index(
        p_device.get_raw_physical_handle(), memory_requirements.memoryTypeBits,
        memory_properties);

    const VkMemoryAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index,
    };

    VkDeviceMemory memory;
    const auto result2 = vkAllocateMemory(p_device.get_raw_handle(),
                                          &allocate_info, nullptr, &memory);
    if (result2 != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to allocate buffer memory. Vulkan error "s +
            std::to_string(result) + '.');
    }

    const auto result3 =
        vkBindBufferMemory(p_device.get_raw_handle(), buffer, memory, 0);
    vulkan_scene_VK_CHECK(result3, "bind the memory to the buffer");

    return {buffer, memory};
}

// Begins a command buffer for one-time use.
VkCommandBuffer begin_one_time_use_cmd_buffer(const Device& p_device)
{
    const auto command_buffer = p_device.allocate_primary_cmd_buffer();

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    const auto result = vkBeginCommandBuffer(command_buffer, &begin_info);
    vulkan_scene_VK_CHECK(result, "begin one time use command buffer");

    return command_buffer;
}

void end_and_submit_command_buffer(const Device& p_device,
                                   VkCommandBuffer p_command_buffer)
{
    auto result = vkEndCommandBuffer(p_command_buffer);
    vulkan_scene_VK_CHECK(result, "end the command buffer for copying buffers");

    const VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                   .pNext = nullptr,
                                   .commandBufferCount = 1,
                                   .pCommandBuffers = &p_command_buffer};

    result = vkQueueSubmit(p_device.get_graphics_queue(), 1, &submit_info,
                           VK_NULL_HANDLE);
    vulkan_scene_VK_CHECK(result,
                          "submit the command buffer for copying buffers");

    result = vkQueueWaitIdle(p_device.get_graphics_queue());
    vulkan_scene_VK_CHECK(result,
                          "wait for the graphics queue to complete operations");

    p_device.free_command_buffer(p_command_buffer);
}

// Copies the contents of one buffer onto another one.
void copy_buffers(const Device& p_device, const VkBuffer p_source,
                  const VkBuffer p_destination, VkDeviceSize p_size)
{
    const auto command_buffer = begin_one_time_use_cmd_buffer(p_device);

    const VkBufferCopy copy_region{
        .srcOffset = 0, .dstOffset = 0, .size = p_size};

    vkCmdCopyBuffer(command_buffer, p_source, p_destination, 1, &copy_region);

    end_and_submit_command_buffer(p_device, command_buffer);
}

void copy_buffer_to_image(const Device& p_device, VkBuffer p_source,
                          VkImage p_destination, uint32_t p_width,
                          uint32_t p_height)
{
    const auto cmd_buffer = begin_one_time_use_cmd_buffer(p_device);

    const VkBufferImageCopy copy_info{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                               .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                               .mipLevel = 0,
                               .baseArrayLayer = 0,
                               .layerCount = 1,
                               },
        .imageOffset = {.x = 0,                                       .y = 0,                                             .z = 0                                                                                              },
        .imageExtent = {.width = p_width,                                 .height = p_height,.depth = 0}
    };

    vkCmdCopyBufferToImage(cmd_buffer, p_source, p_destination,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

    end_and_submit_command_buffer(p_device, cmd_buffer);
}

template <typename T>
void fill_staging_buffer(VkDevice p_device, VkDeviceMemory p_memory,
                         const T* p_data, size_t p_len)
{
    const auto size = static_cast<VkDeviceSize>(p_len * sizeof(T));
    void* gpu_data;

    vkMapMemory(p_device, p_memory, 0, size, 0, &gpu_data);
    std::memcpy(gpu_data, p_data, size);
    vkUnmapMemory(p_device, p_memory);
}

template <typename T>
inline auto s(bool ca, bool cb, T ra, T rb, const char* msg)
{
    if (ca)
        return ra;
    else if (cb)
        return rb;
    else
        throw std::invalid_argument(msg);
}

void transition_image_layout(const Device& p_device, VkImage p_image,
                             VkFormat p_format, VkImageLayout p_old_layout,
                             VkImageLayout p_new_layout)
{
    const auto cmd_buffer = begin_one_time_use_cmd_buffer(p_device);

    const auto is_undefined_to_transfer_dst =
        p_old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        p_new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    const auto is_transfer_dst_to_shader_read =
        p_old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        p_new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    const auto source_stage =
        s(is_undefined_to_transfer_dst, is_transfer_dst_to_shader_read,
             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
             "unsupported layout transitions");

    const auto destination_stage = s(
        is_undefined_to_transfer_dst, is_transfer_dst_to_shader_read,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        "unsupported layout transitions");

    const VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask =
            static_cast<VkAccessFlags>(s(is_undefined_to_transfer_dst, is_transfer_dst_to_shader_read,
                 static_cast<VkAccessFlagBits>(0), VK_ACCESS_TRANSFER_WRITE_BIT,
                 "unsupported layout transitions")),
        .dstAccessMask =
            static_cast<VkAccessFlags>(s(is_undefined_to_transfer_dst, is_transfer_dst_to_shader_read,
                 VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                 "unsupported layout transitions")),
        .oldLayout = p_old_layout,
        .newLayout = p_new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = p_image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd_buffer, source_stage, destination_stage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    end_and_submit_command_buffer(p_device, cmd_buffer);
}

} // namespace

VertexBuffer::VertexBuffer(const Device& p_device,
                           std::span<const Vertex> p_vertices)
    : m_device(p_device)
{
    const auto buffer_size =
        static_cast<VkDeviceSize>(p_vertices.size() * sizeof(Vertex));

    auto [staging_buffer, staging_buffer_memory] =
        create_buffer<BufferType::Staging>(p_device, buffer_size);

    fill_staging_buffer(p_device.get_raw_handle(), staging_buffer_memory,
                        p_vertices.data(), p_vertices.size());

    auto [buffer, memory] =
        create_buffer<BufferType::Vertex>(p_device, buffer_size);
    m_buffer = buffer;
    m_memory = memory;

    copy_buffers(p_device, staging_buffer, buffer, buffer_size);

    vkFreeMemory(p_device.get_raw_handle(), staging_buffer_memory, nullptr);
    vkDestroyBuffer(p_device.get_raw_handle(), staging_buffer, nullptr);
}

VertexBuffer::~VertexBuffer()
{
    vkFreeMemory(m_device.get_raw_handle(), m_memory, nullptr);
    vkDestroyBuffer(m_device.get_raw_handle(), m_buffer, nullptr);
}

IndexBuffer::IndexBuffer(const Device& p_device,
                         std::span<const uint32_t> p_indices)
    : m_device(p_device)
{
    const auto buffer_size =
        static_cast<VkDeviceSize>(p_indices.size() * sizeof(uint32_t));

    auto [staging_buffer, staging_buffer_memory] =
        create_buffer<BufferType::Staging>(p_device, buffer_size);

    fill_staging_buffer(p_device.get_raw_handle(), staging_buffer_memory,
                        p_indices.data(), p_indices.size());

    auto [buffer, memory] =
        create_buffer<BufferType::Index>(p_device, buffer_size);
    m_buffer = buffer;
    m_memory = memory;

    copy_buffers(p_device, staging_buffer, buffer, buffer_size);

    vkFreeMemory(p_device.get_raw_handle(), staging_buffer_memory, nullptr);
    vkDestroyBuffer(p_device.get_raw_handle(), staging_buffer, nullptr);
}

IndexBuffer::~IndexBuffer()
{
    vkFreeMemory(m_device.get_raw_handle(), m_memory, nullptr);
    vkDestroyBuffer(m_device.get_raw_handle(), m_buffer, nullptr);
}

Texture::Texture(const Device& p_device, std::string_view p_file_path)
    : m_device(p_device)
{
    uint8_t* pixels = stbi_load(p_file_path.data(), &m_width, &m_height,
                                &m_channels, STBI_rgb_alpha);
    if (!pixels)
    {
        throw std::runtime_error("Failed to load image "s +
                                 std::string(p_file_path));
    }

    create(pixels);
}

void Texture::create(uint8_t* p_pixels)
{
    const auto image_size =
        static_cast<VkDeviceSize>(m_width * m_height * STBI_rgb_alpha);

    auto [staging_buffer, staging_buffer_memory] =
        create_buffer<BufferType::Staging>(m_device, image_size);
    fill_staging_buffer(m_device.get_raw_handle(), staging_buffer_memory,
                        p_pixels, image_size);

    stbi_image_free(p_pixels);

    const VkImageCreateInfo image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent =
            {
                     .width = static_cast<uint32_t>(m_width),
                     .height = static_cast<uint32_t>(m_height),
                     .depth = 0,
                     },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto result = vkCreateImage(m_device.get_raw_handle(), &image_create_info,
                                nullptr, &m_image);
    vulkan_scene_VK_CHECK(result, "create an image for the texture");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(m_device.get_raw_handle(), m_image,
                                 &memory_requirements);

    const VkMemoryAllocateInfo memory_allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex =
            find_memory_type_index(m_device.get_raw_physical_handle(),
                                   memory_requirements.memoryTypeBits,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

    result = vkAllocateMemory(m_device.get_raw_handle(), &memory_allocate_info,
                              nullptr, &m_memory);
    vulkan_scene_VK_CHECK(result, "allocate memory for an image");

    vkBindImageMemory(m_device.get_raw_handle(), m_image, m_memory, 0);

    transition_image_layout(m_device, m_image, image_create_info.format,
                            image_create_info.initialLayout,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(m_device, staging_buffer, m_image,
                         static_cast<uint32_t>(m_width),
                         static_cast<uint32_t>(m_height));
    transition_image_layout(m_device, m_image, image_create_info.format,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkFreeMemory(m_device.get_raw_handle(), staging_buffer_memory, nullptr);
    vkDestroyBuffer(m_device.get_raw_handle(), staging_buffer, nullptr);
}

Texture::~Texture()
{
    vkDestroyImage(m_device.get_raw_handle(), m_image, nullptr);
    vkFreeMemory(m_device.get_raw_handle(), m_memory, nullptr);
}