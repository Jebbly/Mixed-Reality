#version 330

in vec2 vTexcoord;

out vec4 fragColor;

void main()
{
    fragColor = vec4(vTexcoord, 1.0, 1.0);
}