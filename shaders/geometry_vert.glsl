#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 persp;

out vec3 vPosition;
out vec3 vNormal;

void main()
{
    vec4 world_position = model * vec4(position, 1.0);
    vPosition = world_position.xyz;
    vNormal = transpose(inverse(mat3(model))) * normal;

    gl_Position = persp * view * world_position;
}