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
    // calculate the fixed correct position
	vec3 position = mat3(ubo.model * ubo.view) * in_position;
  	gl_Position = (ubo.proj * vec4( position, 0.0 )).xyzz;

    // pass world coordinate to fragment shader
  	out_uvw = in_position;
}