#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "device.h"

struct application
{
    bool is_running;
    struct device device;
};

void initialise_application(struct application* app, bool enable_validation)
{
    puts("Initialising application.");
    
    create_new_device(&(app->device), "Vulkan Scene", enable_validation);
    
    /* There is no main loop just yet so it exits just after initialisation */
    app->is_running = false;
}

void update_application(struct application* app)
{
    puts("Updating application");
}

void destroy_application(struct application* app)
{
    puts("Destroying application");
    
    destroy_device(&(app->device));
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
    initialise_application(&application, enable_validation);
    
    while (application.is_running)
    {
        update_application(&application);
    }
    
    destroy_application(&application);
}