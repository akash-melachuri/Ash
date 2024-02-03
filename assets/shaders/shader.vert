#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0, set = 0) uniform GlobalBufferObject {
    mat4 view;
    mat4 proj;
} gbo;

layout (binding = 0, set = 2) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 fragPos;
layout (location = 1) out vec4 fragNormal;
layout (location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = gbo.proj * gbo.view * ubo.model * vec4(inPosition, 1.0);
    fragPos = ubo.model * vec4(inPosition, 1.0);
    fragNormal = ubo.model * vec4(inNormal, 1.0);

    fragTexCoord = inTexCoord;
}
