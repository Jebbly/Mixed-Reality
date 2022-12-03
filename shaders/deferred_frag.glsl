#version 330

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

out vec4 fragColor;

void main()
{
    // If the normal has length 0, we can assume that
    // there's not an actual object there.
    vec3 normal = texture(gNormal, vTexcoord).rgb;
    if (length(normal) == 0.0) discard;
    vec3 position = texture(gPosition, vTexcoord).rgb;

    vec3 color = vec3(normal.x * normal.x, normal.y * normal.y, normal.z * normal.z);

    fragColor = vec4(color, 1.0f);
}