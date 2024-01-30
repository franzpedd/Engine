#pragma once

#include "Util/Math.h"
#include <vulkan/vulkan.h>
#include <array>

namespace Cosmos
{
	struct Vertex
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv0 = {};
		glm::vec2 uv1 = {};
		glm::vec4 joint = {};
		glm::vec4 weight = {};
		glm::vec4 color = {};

		// returns it's bindings
		static std::array<VkVertexInputBindingDescription, 1> GetBindingDescriptions()
		{
			std::array<VkVertexInputBindingDescription, 1> bindings = {};
			bindings[0].binding = 0;
			bindings[0].stride = sizeof(Vertex);
			bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindings;
		}

		// returns it's attributes
		static std::array<VkVertexInputAttributeDescription, 7> GetAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 7> attributes = {};

			attributes[0].binding = 0;
			attributes[0].location = 0;
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[0].offset = offsetof(Vertex, position);

			attributes[1].binding = 0;
			attributes[1].location = 1;
			attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[1].offset = offsetof(Vertex, normal);

			attributes[2].binding = 0;
			attributes[2].location = 2;
			attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributes[2].offset = offsetof(Vertex, uv0);

			attributes[3].binding = 0;
			attributes[3].location = 3;
			attributes[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributes[3].offset = offsetof(Vertex, uv1);

			attributes[4].binding = 0;
			attributes[4].location = 4;
			attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[4].offset = offsetof(Vertex, joint);

			attributes[5].binding = 0;
			attributes[5].location = 5;
			attributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[5].offset = offsetof(Vertex, weight);

			attributes[6].binding = 0;
			attributes[6].location = 6;
			attributes[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[6].offset = offsetof(Vertex, color);

			return attributes;
		}
	};
}