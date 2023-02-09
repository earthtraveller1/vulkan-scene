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

    inline static auto get_vertex_binding()
    {
        return VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    inline static auto get_vertex_attributes()
    {
        return std::array{
  // The position attribute
            VkVertexInputAttributeDescription{
                                              .location = 0,
                                              .binding = 0,
                                              .format = VK_FORMAT_R32G32B32_SFLOAT,
                                              .offset = offsetof(Vertex, position)}
        };
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

} // namespace vulkan_scene