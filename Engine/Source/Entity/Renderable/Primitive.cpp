#include "epch.h"
#include "Primitive.h"

namespace Cosmos
{
    Quad::Quad(VKVertex v0, VKVertex v1, VKVertex v2, VKVertex v3)
    {
        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mVertices[3] = v3;
    }

    Quad::~Quad()
    {

    }

    void Quad::OnUpdate(float timestep)
    {

    }

    void Quad::OnRender(VkCommandBuffer commandbuffer, glm::mat4 transform)
    {
        
    }
}