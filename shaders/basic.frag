#version 450

layout (location = 0) out vec4 out_color;

#include "push-constants.glsl"

layout (binding = 0) uniform sampler2D texture_sampler;

layout (location = 0) in vec2 uv;

void main()
{
    out_color = texture(texture_sampler, uv) * vec4(1.0, push_constants.color_shift, push_constants.color_shift, 1.0);
}