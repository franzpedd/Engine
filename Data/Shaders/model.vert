#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform MVP_UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform LIGHT_UBO
{
    vec4 ambient;
    vec4 color;
	vec3 position;
} light;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragColor;
layout(location = 1) out vec2 outFragTexCoord;

void main()
{
    // set vertex position on world
    vec4 positionWorld = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * positionWorld;

    // output variables for the fragment shader
    outFragColor = inColor;
    outFragTexCoord = inTexCoord;
}