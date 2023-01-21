#pragma once

#include <span>

#include "math.hpp"

/**
 * This file will contain all of the encapsulations for all of the different types of buffers, from the vertex buffers to the 
 * uniform buffers. They should all be based on the same fundamental infrastructure.
*/

namespace vulkan_scene
{
struct Vertex
{
    Vector3 position;
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
    VertexBuffer(const std::span<Vertex>& vertices);
    
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
}