#version 450

layout (location = 0) in vec3 a_position;

layout (push_constant) uniform constants
{
    mat4 projection;
} PushConstants;

void main()
{
    gl_Position = vec4(a_position, 1.0);
}