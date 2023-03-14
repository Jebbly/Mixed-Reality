#version 330

layout (location = 0) out vec4 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 diff_spec;

struct Material {
    sampler2D texture_diffuse;
    sampler2D texture_specular;
};

uniform Material material;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in float vDepth;

void main()
{
    position = vec4(vPosition, vDepth);

    normal = normalize(vNormal);

    diff_spec.rgb = texture(material.texture_diffuse, vTexCoord).rgb;
    diff_spec.a = texture(material.texture_specular, vTexCoord).a;
}