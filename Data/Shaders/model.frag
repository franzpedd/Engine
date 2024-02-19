#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform MVP_UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform sampler2D albedoSampler;

layout(location = 0) in vec3 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(albedoSampler, inFragTexCoord);
}