layout (push_constant) uniform constants
{
    // Vertex shader push constants.
    mat4 transform;
    
    // Fragment shader push constants.
    float color_shift;
} push_constants;