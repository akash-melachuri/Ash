#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 fragPos;
layout (location = 1) in vec4 fragNormal;
layout (location = 2) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout (binding = 1, set = 0) uniform LightBufferObject {
    vec4 pos;
    vec4 color;
} lbo;
layout (binding = 0, set = 1) uniform sampler2D diffuseSampler;

void main() {
    float ambientFactor = 0.1;

    vec4 lightDir = normalize(lbo.pos - fragPos);
    float diff = max(dot(fragNormal.xyz, lightDir.xyz), 0.0);
    vec3 diffuse = diff * lbo.color.xyz;

    outColor = vec4((ambientFactor + diffuse) * texture(diffuseSampler, fragTexCoord).xyz, 1.0);
}
