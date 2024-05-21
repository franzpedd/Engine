#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform MVP_UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec3 in_position;

layout (location = 0) out vec3 out_uvw;

void main() 
{
	// set vertex position on world
	//gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0);				// see the cubemap on world
	gl_Position = ubo.proj * ubo.view * mat4(mat3(ubo.view)) * vec4(in_position, 1.0);		// project the cubemap into infinity

	out_uvw = in_position;
}