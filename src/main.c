#include <stdio.h>
#include <stdlib.h>

#include "device.h"
#include "window.h"

int main()
{
    if (!create_device())
        return EXIT_FAILURE;

    if (!create_window())
        return EXIT_FAILURE;

    while (is_window_open())
    {
        update_window();
    }

    destroy_window();
    destroy_device();
}
