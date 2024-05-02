#pragma once

#include "Renderer/Vulkan/VKVertex.h"
#include <array>

namespace Cosmos
{
    class Quad
    {
    public:

        // constructor
        Quad();

        // constructor
        Quad(VKVertex v0, VKVertex v1, VKVertex v2, VKVertex v3);

        // destructor
        ~Quad();

        // returns a reference to the quad vertices
        inline std::array<VKVertex, 4>& GetVerticesRef() { return mVertices; }

    public:

        // updates any quad frame logic
        void OnUpdate(float timestep, glm::mat4 transform);

        // draws the quad
        void OnRender(VkCommandBuffer commandbuffer);

    private:

        std::array<VKVertex, 4> mVertices;
        std::vector<uint16_t> mIndices = { 0, 1, 2, 2, 3, 0 };
    };
}