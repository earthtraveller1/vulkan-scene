#version 450

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform constants
{
    float color_shift_amount;
} PushConstants;

void main()
{
    out_color = vec4(1.0, PushConstants.color_shift_amount, 0.0, 1.0);
}