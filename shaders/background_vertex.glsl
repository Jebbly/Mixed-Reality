#version 330

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texcoord;

out vec2 vTexcoord;

void main()
{
    gl_Position = vec4(pos, 1.0, 1.0);
    vTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
}