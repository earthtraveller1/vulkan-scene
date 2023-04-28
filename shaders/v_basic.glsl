#version 450 

layout (location = 0) in vec3 a_location;

void main()
{
    gl_Position = vec4(a_location, 1.0);
}
