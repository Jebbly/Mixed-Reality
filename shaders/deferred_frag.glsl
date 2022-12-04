#version 330

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D depthTexture;

out vec4 fragColor;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

const int NUM_LIGHTS = 4;
uniform Light lights[NUM_LIGHTS];

const vec3 diffuse = vec3(0.5f, 0.5f, 0.5f);

void main()
{
    // If the normal has length 0, we can assume that
    // there's not an actual object there.
    vec3 normal = texture(gNormal, vTexcoord).rgb;
    if (length(normal) == 0.0) discard;

    vec4 position = texture(gPosition, vTexcoord);
    vec3 world_position = position.xyz;
    float depth = position.w;

    vec2 depth_texcoord = vec2(vTexcoord.x, 1.0f - vTexcoord.y);
    float image_depth = texture(depthTexture, depth_texcoord).r;
    if (image_depth < depth) discard;

    vec3 color = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 light_dir = normalize(lights[i].position - world_position);
        vec3 diffuse = max(dot(normal, light_dir), 0.0f) * diffuse * lights[i].color;

        float distance = length(lights[i].position - world_position);
        color += diffuse / (1.0f + 0.5f * distance);
    }

    fragColor = vec4(color, 1.0f);
}