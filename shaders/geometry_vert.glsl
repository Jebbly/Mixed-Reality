#version 330

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 persp;

void main()
{
    gl_Position = persp * view * model * vec4(pos, 1.0);
}