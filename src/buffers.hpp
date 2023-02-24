#pragma once

#include <span>

#include "math.hpp"

/**
 * This file will contain all of the encapsulations for all of the different
 * types of buffers, from the vertex buffers to the uniform buffers. They should
 * all be based on the same fundamental infrastructure.
 */

namespace vulkan_scene
{
struct Vertex
{
    Vector3f position;
    Vector2f uv;

    constexpr inline static auto get_vertex_binding()
    {
        return VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    constexpr inline static auto get_vertex_attributes()
    {
        return std::array{
            // The position attribute
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position)},
            VkVertexInputAttributeDescription{.location = 1,
                                              .binding = 0,
                                              .format = VK_FORMAT_R32G32_SFLOAT,
                                              .offset = offsetof(Vertex, uv)}};
    }
};

class Device;

/**
 * \brief The vertex buffer.
 */
class VertexBuffer
{
  public:
    // Default constructor makes no sense in this context.
    VertexBuffer() = delete;

    // Constructs from a list of vertices.
    VertexBuffer(const Device& device, std::span<const Vertex> vertices);

    // Disable copying for now.
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Bind it to the command buffer.
    inline void cmd_bind(VkCommandBuffer command_buffer) const
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_buffer, &offset);
    }

    // Destructor
    ~VertexBuffer();

  private:
    // The actual handle to the vertex buffer.
    VkBuffer m_buffer;

    // The handle to the memory.
    VkDeviceMemory m_memory;

    // The handle to the device the buffer is created from.
    const Device& m_device;
};

/**
 * \brief The index buffer.
 */
class IndexBuffer
{
  public:
    // Constructs from a list of indices
    IndexBuffer(const Device& device, const std::span<const uint32_t> indices);

    // Disable copying for now.
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    /**
     * \brief Bind it to the command buffer specified.
     */
    inline void cmd_bind(VkCommandBuffer command_buffer) const
    {
        vkCmdBindIndexBuffer(command_buffer, m_buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    // Destructor
    ~IndexBuffer();

  private:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;

    // Handle to the parent device.
    const Device& m_device;
};

/**
 * \brief An abstraction for a Vulkan image that can be used as a texture.
 */
class Texture
{
  public:
    // Loads the image from disk into the texture clas.
    Texture(const Device& device, std::string_view file_path);

    // Loads the image from memory instead.
    Texture(const Device& device, uint8_t* pixels) : m_device(device)
    {
        create(pixels);
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // The descriptor set layout information
    constexpr static inline VkDescriptorSetLayoutBinding
    get_descriptor_set_layout_binding(uint32_t binding)
    {
        return VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};
    }

    // Information required for the descriptor sets
    inline VkDescriptorImageInfo get_image_info() const
    {
        return VkDescriptorImageInfo{
            .sampler = m_sampler,
            .imageView = m_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    }

    inline VkWriteDescriptorSet
    get_descriptor_write(VkDescriptorSet descriptor_set, uint32_t binding,
                         const VkDescriptorImageInfo* image_info) const
    {
        return VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr};
    }

    ~Texture();

  private:
    // Internal creation function.
    void create(uint8_t* pixels);

    void create_image_view();

    void create_sampler();

    VkImage m_image;
    VkDeviceMemory m_memory;

    VkImageView m_view;
    VkSampler m_sampler;

    int m_width, m_height, m_channels;

    const Device& m_device;
};

} // namespace vulkan_scene