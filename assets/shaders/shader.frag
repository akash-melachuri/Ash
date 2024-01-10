#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec4 fragNormal;

layout (location = 0) out vec4 outColor;

layout (binding = 1, set = 0) uniform sampler2D diffuseSampler;

void main() {
    outColor = texture(diffuseSampler, fragTexCoord);
}
