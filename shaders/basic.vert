#version 450 

#include "push-constants.glsl"

layout (location = 0) in vec3 a_location;

layout (location = 0) out vec3 location;

void main()
{
    gl_Position = vec4(a_location + push_constants.position_shift, 1.0);
    location = a_location;
}
