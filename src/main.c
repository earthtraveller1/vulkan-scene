#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "device.h"
#include "window.h"
#include "swap-chain.h"

struct application
{
    bool is_running;
    struct device device;
    struct window* window;
    struct swap_chain swap_chain;
};

void initialise_application(struct application* app, bool enable_validation, bool* status)
{
    puts("Initialising application.");
    
    app->window = create_window(800, 600, "A Basic Vulkan Scene");
    create_new_device(&(app->device), "Vulkan Scene", enable_validation, app->window, status);
    if (!(*status))
    {
        return;
    }
    
    if (!create_new_swap_chain(&app->swap_chain, &app->device, 800, 600))
    {
        *status = false;
        return;
    }
    
    app->is_running = true;
}

void update_application(struct application* app)
{
    update_window(app->window);
    
    app->is_running = is_window_open(app->window);
}

void destroy_application(struct application* app)
{
    puts("Destroying application");
    
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
    bool app_creation_succeeded;
    initialise_application(&application, enable_validation, &app_creation_succeeded);
    if (!app_creation_succeeded)
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