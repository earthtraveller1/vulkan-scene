#version 450

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform push_constants_t 
{
    float color_shift;
} push_constants;

void main()
{
    out_color = vec4(1.0, 1.0, 0.0 + push_constants.color_shift, 1.0);
}
