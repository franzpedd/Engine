#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 proj;
    mat4 view;
    mat4 model;
    bool selected; // pixel is selected on editor
} ubo;

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(inColor, 1.0);

    if(ubo.selected)
    {
        // edit this to a better looking
        outColor  = vec4(1.0, 0.65, 0, 1.0);
    }
}