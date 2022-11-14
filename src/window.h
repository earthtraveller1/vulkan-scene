#ifndef INCLUDED_WINDOW_H
#define INCLUDED_WINDOW_H

#include <stdint.h>
#include <stdbool.h>

/* A basic window abstraction. Doesn't do much besides what I need for this pr-
oject in particular */

struct window;

/* Creates a window. */
struct window* create_window(uint16_t width, uint16_t height, const char* title);

/* Shows the window. */
void show_window(struct window* window);

/* Checks if the window is still open. */
bool is_window_open(struct window* window);

/* Updates the window. */
void update_window(struct window* window);

/* Destroys the window. */
void destroy_window(struct window* window);

#endif