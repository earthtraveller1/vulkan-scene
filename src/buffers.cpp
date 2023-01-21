#include "device.hpp"

#include "buffers.hpp"

using namespace std::string_literals;

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

template<BufferType buffer_type>
std::tuple<VkBuffer, VkDeviceMemory>
create_buffer(const vulkan_scene::Device& p_device, VkDeviceSize p_size)
{
    const VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_size,
        .usage = [buffer_type]() -> int
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
        buffer_type == BufferType::Staging ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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
    const auto result = vkAllocateMemory(p_device.get_raw_handle(),
                                         &allocate_info, nullptr, &memory);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to allocate buffer memory. Vulkan error "s +
            std::to_string(result) + '.');
    }

    return {buffer, memory};
}
} // namespace