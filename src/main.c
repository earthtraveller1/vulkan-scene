#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "graphics.h"
#include "swapchain.h"
#include "window.h"

int main(int argc, const char* const* const argv)
{
    bool enable_validation = false;

    for (const char* const* arg = argv; arg < argv + argc; arg++)
    {
        if (strcmp(*arg, "--enable-validation") == 0)
            enable_validation = true;
    }

    if (!create_window())
        return EXIT_FAILURE;

    if (!create_device(enable_validation))
        return EXIT_FAILURE;

    if (!create_swap_chain())
        return EXIT_FAILURE;

    VkRenderPass render_pass;
    if (!create_render_pass(&render_pass))
        return EXIT_FAILURE;

    VkFramebuffer* swap_chain_framebuffers;
    uint32_t swap_chain_framebuffer_count;
    if (!create_swap_chain_framebuffers(render_pass, &swap_chain_framebuffers, &swap_chain_framebuffer_count))
        return EXIT_FAILURE;

    struct graphics_pipeline pipeline;
    if (!create_graphics_pipeline("shaders/basic.vert.spv", "shaders/basic.frag.spv", render_pass, &pipeline))
        return EXIT_FAILURE;

    struct vertex vertices[3] = {{{0.0f, -0.5f}}, {{-0.5f, 0.5f}}, {{0.5f, 0.5f}}};

    struct buffer vertex_buffer;
    if (!create_buffer(vertices, sizeof(vertices), BUFFER_TYPE_VERTEX, &vertex_buffer))
        return EXIT_FAILURE;

    VkSemaphore image_available_semaphore;
    if (!create_semaphore(&image_available_semaphore))
        return EXIT_FAILURE;

    VkSemaphore render_done_semaphore;
    if (!create_semaphore(&render_done_semaphore))
        return EXIT_FAILURE;

    VkFence frame_fence;
    if (!create_fence(&frame_fence))
        return EXIT_FAILURE;

    while (is_window_open())
    {
        update_window();
    }

    destroy_buffer(&vertex_buffer);
    destroy_graphics_pipeline(&pipeline);
    destroy_swap_chain_framebuffers(swap_chain_framebuffers, swap_chain_framebuffer_count);
    destroy_render_pass(render_pass);
    destroy_swapchain();
    destroy_window();
    destroy_device();

    return 0;
}
