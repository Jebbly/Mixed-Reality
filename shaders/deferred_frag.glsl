#version 330

in vec2 vTexcoord;

uniform sampler2D backgroundImage;

out vec4 fragColor;

void main()
{
    // Placeholder at the moment
    fragColor = texture(backgroundImage, vTexcoord);
}