#ifndef INCLUDED_RENDERER_H
#define INCLUDED_RENDERER_H

#include <vulkan/vulkan.h>

/**
 * \file A basic abstraction for rendering with the Vulkan API.
*/

/**
 * \brief Start rendering.
*/
void begin_renderer(VkCommandBuffer cmd_buffer);

/* A placeholder function. Right now, I'm just trying to get triangle working
on the screen. */
void draw();

/**
 * \brief Stop rendering.
*/
void end_renderer(VkCommandBuffer cmd_buffer);

#endif