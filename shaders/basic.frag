#version 450

#include "push-constants.glsl"

layout (location = 0) out vec4 out_color;
layout (location = 0) in vec3 location;

void main()
{
    out_color = vec4(1.0, sin(location.x * 10), push_constants.color_shift, 1.0);
}
