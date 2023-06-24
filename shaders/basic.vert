#version 450 

layout (location = 0) in vec3 a_location;

layout (location = 0) out vec3 location;

layout (push_constant) uniform constant
{
    float position_shift;
} push_constants;

void main()
{
    gl_Position = vec4(a_location + push_constants.position_shift, 1.0);
    location = a_location;
}
