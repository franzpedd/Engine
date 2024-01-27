#pragma once

#include "Defines.h"
#include "Primitive.h"
#include "Physics/BoundingBox.h"
#include "Util/Math.h"

#include <vulkan/vulkan.h>

namespace Cosmos
{
	class Renderer;
}

namespace Cosmos::ModelHelper
{
	// forward declarations
	struct Primitive;

	struct Mesh
	{
	public:

		struct UBO
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkDescriptorBufferInfo descriptor = {};
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			void* mapped = nullptr;
		};

		struct UniformBlock
		{
			glm::mat4 matrix = {};
			glm::mat4 jointMatrix[MAX_NUM_JOINTS] = {};
			float jointCount = 0.0f;
		};

	public:

		// constructor
		Mesh(std::shared_ptr<Renderer>& renderer, glm::mat4 matrix);

		// destructor
		~Mesh();

	public:

		// sets the meshe's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

	public:

		std::shared_ptr<Renderer>& renderer;
		std::vector<Primitive*> primitives = {};
		BoundingBox bb = {};
		BoundingBox aabb = {};
		UBO ubo = {};
		UniformBlock uniformBlock = {};
	};
}