#include <cstring>

#include "device.hpp"
#include "utils.hpp"

#include "buffers.hpp"

using namespace std::string_literals;
using vulkan_scene::Device;
using vulkan_scene::VertexBuffer;
using vulkan_scene::IndexBuffer;

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

// Copies the contents of one buffer onto another one.
void copy_buffers(const Device& p_device, const VkBuffer p_source,
                  const VkBuffer p_destination, VkDeviceSize p_size)
{
    const auto command_buffer = p_device.allocate_primary_cmd_buffer();

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    auto result = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to begin recording a command buffer for copying buffers. Vulkan error "s +
            std::to_string(result) + '.');
    }

    const VkBufferCopy copy_region{
        .srcOffset = 0, .dstOffset = 0, .size = p_size};

    vkCmdCopyBuffer(command_buffer, p_source, p_destination, 1, &copy_region);

    result = vkEndCommandBuffer(command_buffer);
    vulkan_scene_VK_CHECK(result, "end the command buffer for copying buffers");

    const VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                   .pNext = nullptr,
                                   .commandBufferCount = 1,
                                   .pCommandBuffers = &command_buffer};

    result = vkQueueSubmit(p_device.get_graphics_queue(), 1, &submit_info,
                           VK_NULL_HANDLE);
    vulkan_scene_VK_CHECK(result,
                          "submit the command buffer for copying buffers");

    result = vkQueueWaitIdle(p_device.get_graphics_queue());
    vulkan_scene_VK_CHECK(result,
                          "wait for the graphics queue to complete operations");

    p_device.free_command_buffer(command_buffer);
}

template<typename T>
void fill_staging_buffer(VkDevice p_device, VkDeviceMemory p_memory, const T* p_data, size_t p_len)
{
    const auto size = static_cast<VkDeviceSize>(p_len * sizeof(T));
    void* gpu_data;

    vkMapMemory(p_device, p_memory, 0, size, 0, &gpu_data);
    std::memcpy(gpu_data, p_data, size);
    vkUnmapMemory(p_device, p_memory);
}
} // namespace

VertexBuffer::VertexBuffer(const Device& p_device, std::span<const Vertex> p_vertices)
    : m_device(p_device)
{
    const auto buffer_size =
        static_cast<VkDeviceSize>(p_vertices.size() * sizeof(Vertex));

    auto [staging_buffer, staging_buffer_memory] =
        create_buffer<BufferType::Staging>(p_device, buffer_size);

    fill_staging_buffer(p_device.get_raw_handle(), staging_buffer_memory, p_vertices.data(), p_vertices.size());

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

IndexBuffer::IndexBuffer(const Device& p_device, std::span<const uint32_t> p_indices)
    : m_device(p_device)
{
    const auto buffer_size = static_cast<VkDeviceSize>(p_indices.size() * sizeof(uint32_t));
    
    auto [staging_buffer, staging_buffer_memory] = create_buffer<BufferType::Staging>(p_device, buffer_size);
    
    fill_staging_buffer(p_device.get_raw_handle(), staging_buffer_memory, p_indices.data(), p_indices.size());
    
    auto [buffer, memory] = create_buffer<BufferType::Index>(p_device, buffer_size);
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