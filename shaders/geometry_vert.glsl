#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 local;
uniform mat4 plane;
uniform mat4 view;
uniform mat4 persp;

out vec3 vPosition;
out vec3 vNormal;
out float vDepth;

void main()
{
    mat4 model = plane * local;
    vec4 world_position = model * vec4(position, 1.0);
    vPosition = world_position.xyz;
    vNormal = transpose(inverse(mat3(model))) * normal;

    vec4 camera_position = view * world_position;
    vDepth = camera_position.z;

    gl_Position = persp * camera_position;
}