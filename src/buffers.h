#ifndef INCLUDED_VERTEX_BUFFER_H
#define INCLUDED_VERTEX_BUFFER_H

#include <vulkan/vulkan.h>

#include "device.h"
#include "math.h"

/* An abstraction for working with Vulkan vertex buffers, and some utility fun-
ctions for stuff like vertex input. */

struct vertex
{
    struct vector_3 position;
};

#define VERTEX_ATTRIBUTE_DESCRIPTION_COUNT 1

extern const VkVertexInputBindingDescription VERTEX_BINDING_DESCRIPTION;

extern const VkVertexInputAttributeDescription
    VERTEX_ATTRIBUTE_DESCRIPTIONS[VERTEX_ATTRIBUTE_DESCRIPTION_COUNT];

enum buffer_type
{
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_INDEX
};

struct buffer
{
    VkBuffer buffer;
    VkDeviceMemory memory;

    enum buffer_type type;

    const struct device* device;
};

/**
 * \brief Creates a vertex buffer.
 *
 * Creates and allocates a vertex buffer from the device that was passed as a
 * parameter, and then copies the `data` into the buffer on the GPU. Please note
 * that if `data_len` is greater than the actual length of the data, it will
 * result in undefined behaviour!
 *
 * When the function fails, the error message will be printed to stderr, and a
 * return value of `false` will indicate failure to the caller.
 *
 * \param buffer The vertex buffer that the resulting object will be written to.
 * \param device The device that the vertex buffer will be created from.
 * \param data Pointer to the data that will be copied into the vertex buffer.
 * \param data_len Length of the data, in number of
 *
 * \returns `true` if the function succeeds, `false` if the function fails.
 */
bool create_vertex_buffer(struct buffer* buffer, const struct device* device,
                          const struct vertex* data, size_t data_len);

/**
 * \brief Creates an index buffer. Parameters are the same as the ones for the
 * `create_vertex_buffer`.
 */
bool create_index_buffer(struct buffer* buffer, const struct device* device,
                         const uint32_t* data, size_t data_len);

/**
 * \brief Binds the specified buffer so that it can be used for rendering.
 * 
 * \param self The buffer to bind.
 * \param cmd_buffer The command buffer to bind to.
*/
void bind_buffer(const struct buffer* self, VkCommandBuffer cmd_buffer);

/**
 * \brief Destroys the buffer.
 *
 * \param buffer The buffer to destroy.
 */
void destroy_buffer(struct buffer* buffer);

#endif