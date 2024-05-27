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
	// doesnt move
	// convert cubemap coordinates into vulkan coordinate space
	//out_uvw = in_position;
	//out_uvw.xy *= -1.0f;
	//
	// remove translation from view matrix
	//mat4 viewMat = mat4(mat3(ubo.model));
	//gl_Position = ubo.proj * viewMat * vec4(in_position.xyz, 1.0);

	// moves
	vec3 position = mat3(ubo.model * ubo.view) * in_position;
  	gl_Position = (ubo.proj * vec4( position, 0.0 )).xyzz;
  	out_uvw = in_position;
	
}