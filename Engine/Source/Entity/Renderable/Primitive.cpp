#include "epch.h"
#include "Primitive.h"

namespace Cosmos
{
    Quad::Quad()
    {
        VKVertex v0, v1, v2, v3;

        // bottom left
        v0.position = { -0.5f, -0.5f, 0.0f };
        v0.color = { 1.0f, 1.0f, 1.0f };
        v0.uv0 = { 0.0f, 0.0f };

        // bottom right
        v0.position = { 0.5f, -0.5f, 0.0f };
        v0.color = { 1.0f, 1.0f, 1.0f };
        v0.uv0 = { 1.0f, 0.0f };

        // top right
        v0.position = { 0.5f,  0.5f, 0.0f };
        v0.color = { 1.0f, 1.0f, 1.0f };
        v0.uv0 = { 1.0f, 1.0f };

        // top left
        v0.position = { -0.5f,  0.5f, 0.0f };
        v0.color = { 1.0f, 1.0f, 1.0f };
        v0.uv0 = { 0.0f, 1.0f };

        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mVertices[3] = v3;
    }

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

    void Quad::OnUpdate(float timestep, glm::mat4 transform)
    {

    }

    void Quad::OnRender(VkCommandBuffer commandbuffer)
    {

    }
}