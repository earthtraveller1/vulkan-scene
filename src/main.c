#include <stdio.h>

#include "device.h"
#include "window.h"

int main()
{
    create_device();

    create_window();

    while (is_window_open())
    {
        update_window();
    }

    destroy_window();
    destroy_device();
}
