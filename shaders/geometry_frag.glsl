#version 330

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;

in vec3 vPosition;
in vec3 vNormal;

void main()
{
    position = vPosition;
    normal = normalize(vNormal);
}