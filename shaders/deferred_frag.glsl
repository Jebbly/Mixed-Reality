#version 330

in vec2 vTexcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffSpec;
uniform sampler2D depthTexture;

out vec4 fragColor;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

const int NUM_LIGHTS = 4;
uniform Light lights[NUM_LIGHTS];

uniform vec3 viewPos;

void main()
{
    vec4 position = texture(gPosition, vTexcoord);
    vec3 normal = texture(gNormal, vTexcoord).rgb;
    vec4 diff_spec = texture(gDiffSpec, vTexcoord, rgb);

    // If the normal has length 0, we can assume that
    // there's not an actual object there.
    if (length(normal) == 0.0) discard;

    // Check if the virtual object should be occluded
    // by any real objects or not.
    float depth = position.w;
    float image_depth = texture(depthTexture, vec2(vTexcoord.x, 1.0f - vTexcoord.y)).r;
    if (image_depth < depth) discard;

    // If the virtual object is un-occluded, we proceed with shading.
    vec3 world_pos = position.xyz;
    vec3 diffuse = diff_spec.rgb;
    float specularity = diff_spec.a;
    vec3 view_dir = normalize(viewPos - world_pos);

    // Add up the contributions from each light
    vec3 color = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 light_dir = normalize(lights[i].position - world_pos);
        float light_dist = length(lights[i].position - world_pos);

        // Diffuse component
        float diffuse_value = max(dot(normal, light_dir), 0.0f);
        vec3 diffuse_color = diffuse_value * diffuse * lights[i].color * attenuation;

        // Specular component
        vec3 half_dir = normalize(light_dir + view_dir);
        float specular_value = pow(max(dot(normal, half_dir), 0.0f), 16.0f);
        vec3 specular_color = specular_value * specularity * lights[i].color * attenuation;

        // Add components with attenuation
        float attenuation = 1.0f / (1.0f + 0.5f * light_dist);
        color += (diffuse_color + specular_color) * attenuation;
    }

    fragColor = vec4(color, 1.0f);
}