#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"
#include "window.h"


#define WWIDTH 800
#define WHEIGHT 600

struct application
{
    bool is_running;
    struct window* window;
    struct renderer renderer;
};

bool initialise_application(struct application* app, bool enable_validation)
{
    puts("Initialising application.");

    app->window = create_window(WWIDTH, WHEIGHT, "A Basic Vulkan Scene");

    if (!create_new_renderer(&app->renderer, app->window,
                             "A Basic Vulkan Scene", enable_validation,
                             "shaders/basic.vert.spv",
                             "shaders/basic.frag.spv"))
    {
        fputs("[ERROR]: Failed to create the renderer.\n", stderr);
        return false;
    }
    
    const struct vertex vertices[3] = {
        { -0.5f,  0.5f, 0.0f },
        {  0.5f,  0.5f, 0.0f },
        {  0.0f, -0.5f, 0.0f }
    };
    
    if (!load_vertex_data_into_renderer(&app->renderer, 3, vertices))
    {
        fputs("[ERROR]: Failed to laod the vertex data into the renderer.\n", stderr);
        return false;
    }

    app->is_running = true;

    return true;
}

void update_application(struct application* app)
{
    app->is_running = is_window_open(app->window);

    update_window(app->window);
}

void destroy_application(struct application* app)
{
    puts("Destroying application");

    destroy_renderer(&app->renderer);
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
