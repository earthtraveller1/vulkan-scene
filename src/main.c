#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "graphics-pipeline.h"
#include "swap-chain.h"
#include "window.h"
#include "vertex-buffer.h"
#include "framebuffer-manager.h"

#define WWIDTH 800
#define WHEIGHT 600

struct application
{
    bool is_running;
    struct window* window;

    struct device device;
    struct swap_chain swap_chain;
    struct graphics_pipeline graphics_pipeline;
    struct framebuffer_manager framebuffer_manager;
    
    struct vertex_buffer vertex_buffer;
};

bool initialise_application(struct application* app, bool enable_validation)
{
    puts("Initialising application.");

    app->window = create_window(WWIDTH, WHEIGHT, "A Basic Vulkan Scene");

    bool status;
    create_new_device(&(app->device), "Vulkan Scene", enable_validation,
                      app->window, &status);
    if (!status)
    {
        return false;
    }

    if (!create_new_swap_chain(&app->swap_chain, &app->device, WWIDTH, WHEIGHT))
    {
        return false;
    }

    if (!create_new_graphics_pipeline(
            &app->graphics_pipeline, &app->device, &app->swap_chain,
            "shaders/basic.vert.spv", "shaders/basic.frag.spv"))
    {
        return false;
    }
    
    const struct vertex vertices[3] = {
        {{ 0.0f, 0.5f, 0.0f }},
        {{ 0.5f, -0.5f, 0.0f }},
        {{ -0.5f, -0.5f, 0.0f }}
    };
    
    if (!create_vertex_buffer(&app->vertex_buffer, &app->device, vertices, 3))
    {
        fputs("[ERROR]: Failed to create a vertex buffer!\n", stderr);
        return false;
    }
    
    if (!create_new_framebuffer_manager(&app->framebuffer_manager, &app->swap_chain, &app->graphics_pipeline))
    {
        fputs("[ERROR]: Failed to create the framebuffer manager.\n", stderr);
        return false;
    }

    app->is_running = true;

    return true;
}

void update_application(struct application* app)
{
    update_window(app->window);

    app->is_running = is_window_open(app->window);
}

void destroy_application(struct application* app)
{
    puts("Destroying application");
    
    destroy_framebuffer_manager(&app->framebuffer_manager);
    destroy_vertex_buffer(&app->vertex_buffer);
    destroy_graphics_pipeline(&app->graphics_pipeline);
    destroy_swap_chain(&app->swap_chain);
    destroy_device(&(app->device));
    destroy_window(app->window);
}

int main(int argc, char** argv)
{
    bool enable_validation;
    if (argc > 1)
    {
        if (strcmp(argv[1], "--enable-validation") == 0)
        {
            enable_validation = true;
        }
        else
        {
            enable_validation = false;
        }
    }
    else
    {
        enable_validation = false;
    }

    struct application application;
    if (!initialise_application(&application, enable_validation))
    {
        fputs("[FATAL ERROR]: Failed to initialise the application.\n", stderr);
        return EXIT_FAILURE;
    }

    while (application.is_running)
    {
        update_application(&application);
    }

    destroy_application(&application);
}
