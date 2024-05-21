#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 1) uniform samplerCube sampler_cubemap;

layout (location = 0) in vec3 in_uvw;

layout (location = 0) out vec4 out_fragment_color;

void main() 
{
	out_fragment_color = texture(sampler_cubemap, in_uvw);
}