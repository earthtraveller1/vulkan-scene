#version 450

layout (binding = 0) uniform uniform_buffer_t
{
    float color_offset;
} uniform_buffer;

layout (location = 0) in vec3 a_position;

void main()
{
    gl_Position = vec4(a_position.x + uniform_buffer.color_offset, a_position.y, a_position.z, 1.0);
}
