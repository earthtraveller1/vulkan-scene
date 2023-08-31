#version 450

layout (binding = 0) uniform uniform_buffer_t
{
    mat4 view;
    mat4 projection;

    float color_offset;
} uniform_buffer;

layout (push_constant) uniform push_constants_t 
{
    mat4 model;
} push_constants;

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;

layout (location = 0) out vec2 uv;

void main()
{
    gl_Position = uniform_buffer.projection * uniform_buffer.view * push_constants.model * vec4(a_position.x, a_position.y, a_position.z, 1.0);
    uv = a_uv;
}
