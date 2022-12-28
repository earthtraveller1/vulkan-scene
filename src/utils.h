#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H

#include <vulkan/vulkan.h>

/* A set of common utilities that might be used that doesn't fall into any par-
ticular category. */

#define UNUSED(x) (void)(x)

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

#ifdef VULKAN_SCENE_PROFILE
#include <time.h>
#define PROFILE_INIT clock_t start = (clock() / CLOCKS_PER_MS)
#define PROFILE_PRINT(msg)                                                     \
    printf("[PROFILER]: " msg " took %ld ms.\n", (clock() / CLOCKS_PER_MS) - start);              \
    start = (clock() / CLOCKS_PER_MS)
#define PROFILE_END fputs("\033[1H", stdout)
#else
#define PROFILE_INIT
#define PROFILE_PRINT
#define PROFILE_END
#endif

#define clear_console fputs("\033[2J", stdout)

/* Gets the size of the file. Works best when the file is open in binary mode.
 */
size_t get_file_size(FILE* file);

/* Begins a render pass. */
void begin_render_pass(float clear_color_r, float clear_color_g,
                       float clear_color_b, float clear_color_a,
                       VkCommandBuffer command_buffer,
                       const struct graphics_pipeline* pipeline,
                       const struct swap_chain* swap_chain,
                       const struct framebuffer_manager* framebuffers,
                       uint32_t image_index);

/* Creates a buffer with the specified type and stuff. */
bool create_vulkan_buffer(VkDeviceSize buffer_size, VkBufferUsageFlagBits usage,
                          VkMemoryPropertyFlags memory_flags,
                          const struct device* device, VkBuffer* buffer,
                          VkDeviceMemory* memory);

#endif