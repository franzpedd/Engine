#pragma once

#include "Vertex.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class Renderer;

	class Mesh
	{
	public:

		// constructor
		Mesh(std::shared_ptr<Renderer>& renderer, std::vector<Vertex> vertices, std::vector<uint32_t> indices);

		// destructor
		~Mesh() = default;

	public:

		// returns a reference to the vertices vector
		inline std::vector<Vertex>& GetVerticesRef() { return mVertices; }

		// returns a reference to the indices vector
		inline std::vector<uint32_t>& GetIndicesRef() { return mIndices; }

		// returns the vertex buffer
		inline VkBuffer GetVertexBuffer() const { return mVertexBuffer; }

		// returns the vertex memory
		inline VkDeviceMemory GetVertexMemory() const { return mVertexMemory; }

		// returns the index buffer
		inline VkBuffer GetIndexBuffer() const { return mVertexBuffer; }

		// returns the index memory
		inline VkDeviceMemory GetIndexMemory() const { return mVertexMemory; }

	public:

		// draws the mesh
		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet& descriptorSet);

		// creates the renderer resources for this mesh
		void CreateResources();

		// free used resources used by this mesh
		void DestroyResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;

		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexMemory = VK_NULL_HANDLE;
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexMemory = VK_NULL_HANDLE;
	};
}