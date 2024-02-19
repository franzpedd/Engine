#version 450

layout(binding = 0) uniform MVP_UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 1) in vec3 inNearPoint;
layout(location = 2) in vec3 inFarPoint;

layout(location = 0) out vec4 outColor;

// constants for near and far may be enough
const float cNear = 0.1;
const float cFar = 256.0;

vec4 Grid(vec3 pos, float scale)
{
    vec2 coord = pos.xz * scale;
    vec2 coord_dxdy = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / coord_dxdy;
    float line = min(grid.x, grid.y);
    float minimumz = min(coord_dxdy.y, 1);
    float minimumx = min(coord_dxdy.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    // z axis
    if(pos.x > -0.1 * minimumx && pos.x < 0.1 * minimumx) 
    {
        color.z = 1.0;
    }

    // x axis
    if(pos.z > -0.1 * minimumz && pos.z < 0.1 * minimumz)
    {
        color.x = 1.0;
    }
       
    return color;
}

// calculates depth for the fragment
float ComputeDepth(vec3 pos)
{
    vec4 clipSpace = ubo.proj * ubo.view * vec4(pos.xyz, 1.0);

    return (clipSpace.z / clipSpace.w);
}

// calculates the alpha of the fragment, for line fadding
float ComputeLinearDepth(vec3 pos)
{
    // calculates depth
    vec4 clipSpace = ubo.proj * ubo.view * vec4(pos.xyz, 1.0);

    // put back between -1 and 1
    float clipDepth = (clipSpace.z / clipSpace.w) * 2.0 - 1.0; 

    // get linear value between near and far
    float linearDepth = (2.0 * cNear * cFar) / (cFar + cNear - clipDepth * (cFar - cNear)); 

    // normalize
    return linearDepth / cFar; 
}

void main()
{
    // line parametric equation
    float t = -inNearPoint.y / (inFarPoint.y - inNearPoint.y);

    // fragment 3d position
    vec3 pos = inNearPoint + t * (inFarPoint - inNearPoint);

    gl_FragDepth = ComputeDepth(pos);

    float linearDepth = ComputeLinearDepth(pos);
    float fading = max(0, (0.5 - linearDepth));

    outColor = (Grid(pos, 10) + Grid(pos, 1)) * float(t > 0); // adding multiple resolution for the grid
    outColor.a *= fading;

    if(outColor.w <= 000000.1) discard; // discarting pixels outside grid
}