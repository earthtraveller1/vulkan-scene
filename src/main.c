#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "renderer.h"
#include "utils.h"
#include "window.h"

#define WWIDTH 800
#define WHEIGHT 600

struct application
{
    bool is_running;
    struct window* window;
    struct renderer renderer;

    bool recreate_swap_chain;
};

void draw_application(struct application* app);

void on_window_resize(void* user_pointer, uint16_t width, uint16_t height)
{
    UNUSED(width);
    UNUSED(height);

    struct application* self = (struct application*)user_pointer;

    recreate_renderer_swap_chain(&self->renderer);
    draw_application(self);
}

bool initialise_application(struct application* app, bool enable_validation)
{
    puts("Initialising application.");

    app->window = create_window(WWIDTH, WHEIGHT, "A Basic Vulkan Scene", app);

    set_window_resize_callback(app->window, on_window_resize);

    if (!create_new_renderer(&app->renderer, app->window,
                             "A Basic Vulkan Scene", enable_validation,
                             "shaders/basic.vert.spv",
                             "shaders/basic.frag.spv"))
    {
        fputs("[ERROR]: Failed to create the renderer.\n", stderr);
        return false;
    }

    const struct vertex vertices[4] = {
        {0.5f, -0.5f, -2.0f},
        {0.5f, 0.5f, -2.0f},
        {-0.5f, 0.5f, -2.0f},
        {-0.5f, -0.5f, -2.0f},
    };

    const uint32_t indices[6] = {0, 1, 2, 3, 0, 2};

    if (!load_vertex_data_into_renderer(&app->renderer, 4, vertices))
    {
        fputs("[ERROR]: Failed to laod the vertex data into the renderer.\n",
              stderr);
        return false;
    }

    if (!load_indices_into_renderer(&app->renderer, 6, indices))
    {
        fputs("[ERROR]: Failed to load indices into the renderer.\n", stderr);
        return false;
    }

    app->is_running = true;

    return true;
}

void draw_application(struct application* app)
{
    begin_renderer(&app->renderer, &app->recreate_swap_chain);
    if (app->recreate_swap_chain)
        return;

    struct pipeline_push_constants_f push_constants_f;

    push_constants_f.color_shift_amount =
        fabs(sin(((double)clock() / CLOCKS_PER_MS) / 1000.0));

    uint16_t window_width, window_height;
    get_window_size(app->window, &window_width, &window_height);

    struct pipeline_push_constants_v push_constants_v;
    /* push_constants_v.projection = perspective_projection_matrix(
        -((float)window_width), (float)window_width, -((float)window_height),
        (float)window_height, 100.0f, 0.1f); */

    push_constants_v.projection = perspective_projection_matrix(
        (float)WWIDTH / (float)WHEIGHT, deg2rad(45.0f), 10000.0f, 0.1f);

    draw_polygon(&app->renderer, 6, &push_constants_v, &push_constants_f);

    end_renderer(&app->renderer, &app->recreate_swap_chain);
    if (app->recreate_swap_chain)
        return;
}

void update_application(struct application* app)
{
    app->is_running = is_window_open(app->window);
    draw_application(app);
    update_window(app->window);
    if (app->recreate_swap_chain && app->is_running)
    {
        recreate_renderer_swap_chain(&app->renderer);
    }
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
