#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
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

    while (is_window_open())
    {
        update_window();
    }

    destroy_window();
    destroy_device();
}
