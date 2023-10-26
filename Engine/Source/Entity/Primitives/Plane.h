#pragma once

#include "Core/Camera.h"
#include "Entity/Entity.h"
#include "Util/Math.h"
#include "Util/Random.h"

#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"
#include "Renderer/Vulkan/VKPipeline.h"

#include <vector>

namespace Cosmos
{
	// forward declarations
	class Renderer;

	class Plane : public Entity
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 tint;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions()
			{
				std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

				bindingDescriptions[0].binding = 0;
				bindingDescriptions[0].stride = sizeof(Vertex);
				bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescriptions;
			}

			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
			{
				std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, position);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, tint);

				return attributeDescriptions;
			}
		};

	public:

		// constructor
		Plane(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		~Plane() = default;

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDrestroy() override;

	private:

		// creates graphics pipeline if not already created
		void CreatePipeline();

		// creates both vertex and index buffers
		void CreateBuffers();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;

		//ID mID; // bugged, must fix static 

		std::shared_ptr<VKGraphicsPipeline> mGraphicsPipeline;
		std::shared_ptr<VKBuffer> mVertexBuffer;
		std::shared_ptr<VKBuffer> mIndexBuffer;

		std::vector<Vertex> mVertices;
		std::vector<uint16_t> mIndices;
	};
}