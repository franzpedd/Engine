#pragma once

#include "VKDefines.h"
#include "Util/Math.h"
#include <array>

namespace Cosmos
{
    struct VKVertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 uv0;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescription()
        {
            std::vector<VkVertexInputBindingDescription> bindingDescription(1);
            bindingDescription[0].binding = 0;
            bindingDescription[0].stride = sizeof(VKVertex);
            bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(VKVertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(VKVertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(VKVertex, uv0);

            return attributeDescriptions;
        }

        bool operator==(const VKVertex& other) const
        {
            return position == other.position 
                && color == other.color 
                && uv0 == other.uv0;
        }
    };
}