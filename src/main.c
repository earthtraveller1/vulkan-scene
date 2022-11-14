#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct application
{
    bool is_running;
};

void initialise_application(struct application* app)
{
    puts("Initialising application.");
    app->is_running = true; // Because why not?
}

void update_application(struct application* app)
{
    printf("Updating application %d\n", app->is_running);
}

void destroy_application(struct application* app)
{
    printf("Destroying application %d\n", app->is_running);
}

int main()
{
    struct application application;
    initialise_application(&application);
    
    while (application.is_running)
    {
        update_application(&application);
    }
    
    destroy_application(&application);
}