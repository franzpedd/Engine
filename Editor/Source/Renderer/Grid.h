#pragma once

#include <Engine.h>
#include <array>

namespace Cosmos
{
	// forward declarations
	class Viewport;

	class Grid : public Entity
	{
	public:

		struct Vertex
		{
			math::Vec2 pos;
			math::Vec3 color;

			static std::array<VkVertexInputBindingDescription, 1> GetBindingDescription()
			{
				std::array<VkVertexInputBindingDescription, 1> bindingDescription = {};
				bindingDescription[0].binding = 0;
				bindingDescription[0].stride = sizeof(Vertex);
				bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}
		};

		struct UniformBufferObject
		{
			math::Mat4 model;
			math::Mat4 view;
			math::Mat4 proj;
		};

	public:

		// constructor
		Grid(std::shared_ptr<Renderer>& renderer, Viewport& viewport);

		// destructor
		virtual ~Grid();

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate() override;

		// called before destructor, for freeing resources
		virtual void OnDrestroy() override;

	private:

		// create all used resources
		void CreateResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Viewport& mViewport;
		
		std::shared_ptr<VKBuffer> mVertexBuffer;
		std::shared_ptr<VKBuffer> mIndexBuffer;
		std::shared_ptr<VKShader> mVertexShader;
		std::shared_ptr<VKShader> mFragmentShader;

		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

		const std::vector<Vertex> mVertices =
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint16_t> mIndices = { 0, 1, 2, 2, 3, 0 };
	};
}