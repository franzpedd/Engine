#pragma once

#include "Entity/Entity.h"
#include "Entity/Camera.h"
#include "Renderer/Renderer.h"
#include "Util/Math.h"

#include <vector>

namespace Cosmos
{
	// forward declarations
	// forward declarations
	class Renderer;
	class VKGraphicsPipeline;
	class VKShader;
	class VKBuffer;

	class Plane : public Entity
	{
	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;

			// returns the binding of a cube
			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions()
			{
				std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

				bindingDescriptions[0].binding = 0;
				bindingDescriptions[0].stride = sizeof(Vertex);
				bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescriptions;
			}

			// returns the attributes of a plane
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
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}
		};

	public:

		// constructor
		Plane(Scene* scene, std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		virtual ~Plane() = default;

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnRenderDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDestroy() override;

	private:

		// creates graphics pipeline if not already created
		void CreatePipeline();

		// creates both vertex and index buffers
		void CreateBuffers();

	private:

		Scene* mScene;
		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;

		std::shared_ptr<VKGraphicsPipeline> mGraphicsPipeline;
		std::shared_ptr<VKBuffer> mVertexBuffer;
		std::shared_ptr<VKBuffer> mIndexBuffer;

		std::vector<Vertex> mVertices;
		std::vector<uint16_t> mIndices;
	};
}