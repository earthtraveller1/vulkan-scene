#version 450

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform push_constants_t 
{
    float color_shift;
} push_constants;

layout (binding = 1) uniform sampler2D texture_sampler;

layout (location = 0) in vec2 uv;

void main()
{
    out_color = vec4(1.0, 1.0, 0.0 + push_constants.color_shift, 1.0) * texture(texture_sampler, uv);
}
