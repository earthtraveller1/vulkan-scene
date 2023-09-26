#version 450

layout (location = 0) out vec4 out_color;

layout (binding = 1) uniform sampler2D texture_sampler;

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 normal;

void main()
{
    float diffuse_factor = max(dot(normalize(normal), vec3(1.0, 0.0, 0.0)), 0.01);
    out_color = diffuse_factor * vec4(1.0, 1.0, 0.0, 1.0) * texture(texture_sampler, uv);
}
