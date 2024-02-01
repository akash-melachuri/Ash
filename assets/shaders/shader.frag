#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec4 fragNormal;

layout (location = 0) out vec4 outColor;

layout (binding = 0, set = 1) uniform sampler2D diffuse;

void main() {
    float ambientFactor = 1.0;

    outColor = ambientFactor * texture(diffuse, fragTexCoord);
}
