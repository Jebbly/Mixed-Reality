#version 330

layout (location = 0) out vec4 position;
layout (location = 1) out vec3 normal;

in vec3 vPosition;
in vec3 vNormal;
in float vDepth;

void main()
{
    position = vec4(vPosition, vDepth);
    normal = normalize(vNormal);
}