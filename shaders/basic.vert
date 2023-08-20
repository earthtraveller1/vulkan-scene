#version 450

layout (binding = 0) uniform uniform_buffer_t
{
    mat4 view;
    mat4 projection;

    float color_offset;
} uniform_buffer;

layout (location = 0) in vec3 a_position;

void main()
{
    gl_Position = uniform_buffer.projection * uniform_buffer.view * vec4(a_position.x, a_position.y, a_position.z, 1.0);
}
