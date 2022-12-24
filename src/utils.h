#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H

#include <vulkan/vulkan.h>

/* A set of common utilities that might be used that doesn't fall into any par-
ticular category. */

#define UNUSED(x) (void)(x)

/* Gets the size of the file. Works best when the file is open in binary mode.
 */
size_t get_file_size(FILE* file);

/* Begins a render pass. */
bool begin_render_pass(float clear_color_r, float clear_color_g,
                       float clear_color_b, float clear_color_a,
                       VkCommandBuffer command_buffer,
                       const struct graphics_pipeline* pipeline,
                       const struct swap_chain* swap_chain,
                       const struct framebuffer_manager* framebuffers,
                       uint32_t image_index);

#endif