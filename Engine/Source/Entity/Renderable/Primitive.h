#pragma once

#include "Renderer/Vulkan/VKVertex.h"
#include <array>

namespace Cosmos
{
    class Quad
    {
    public:

        // constructor
        Quad(VKVertex v0, VKVertex v1, VKVertex v2, VKVertex v3);

        // destructor
        ~Quad();

        // returns a reference to the quad vertices
        inline std::array<VKVertex, 4>& GetVerticesRef() { return mVertices; }

    public:

        // updates any quad frame logic
        void OnUpdate(float timestep);

        // draws the quad
        void OnRender(VkCommandBuffer commandbuffer, glm::mat4 transform);

    private:

        std::array<VKVertex, 4> mVertices;
    };
}