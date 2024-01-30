#pragma once

#include "Math.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Cosmos
{
	// forward declaration
	class Renderer;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv0;
		glm::vec3 normal;
		glm::vec3 color;

		glm::vec3 tangent;
		glm::vec3 bitangent;

		// returns the vertex's bindings
		static std::array<VkVertexInputBindingDescription, 1> GetBindingDescription()
		{
			std::array< VkVertexInputBindingDescription, 1> bindings = {};
			bindings[0].binding = 0;
			bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindings[0].stride = sizeof(Vertex);

			return bindings;
		}

		// returns the vertex's attributes
		static std::array<VkVertexInputAttributeDescription, 4> GetBindingAttributes()
		{
			std::array<VkVertexInputAttributeDescription, 4> attributes = {};

			attributes[0].binding = 0;
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[0].location = 0;
			attributes[0].offset = offsetof(Vertex, position);

			attributes[1].binding = 0;
			attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributes[1].location = 1;
			attributes[1].offset = offsetof(Vertex, uv0);

			attributes[2].binding = 0;
			attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[2].location = 2;
			attributes[2].offset = offsetof(Vertex, normal);

			attributes[3].binding = 0;
			attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[3].location = 3;
			attributes[3].offset = offsetof(Vertex, color);

			return attributes;
		}
	};

	class MeshLoader
	{
	public:

		typedef enum VertexLayout
		{
			VERTEX_LAYOUT_POSITION	= 0x0,
			VERTEX_LAYOUT_UV0		= 0x1,
			VERTEX_LAYOUT_NORMAL	= 0x2,
			VERTEX_LAYOUT_COLOR		= 0x3,
			VERTEX_LAYOUT_TANGENT	= 0x4,
			VERTEX_LAYOUT_BITANGENT = 0x5
			
		} VertexLayout;

		struct MeshEntry
		{
			uint32_t indicesCount = 0;
			uint32_t materialIndex = 0;
			uint32_t vertexBase = 0;
			std::vector<Vertex> vertices = {};
			std::vector<unsigned int> indices = {};
		};

		struct Dimension
		{
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
			glm::vec3 size = glm::vec3(0.0f);
		};

	public:

		// constructor
		MeshLoader(std::shared_ptr<Renderer>& renderer);

		// destructor
		~MeshLoader();

		// returns the mesh vertex buffer
		inline VkBuffer& GetVertexBuffer() { return mVertexBuffer; }

		// returns the mesh index buffer
		inline VkBuffer& GetIndexBuffer() { return mIndexBuffer; }

		// returns how many indices the mesh has
		inline uint32_t GetIndexCount() { return mIndexCount; }

	public:

		// loads a mesh given it's path
		void LoadMesh(std::string path);

		// creates all resources used to create the mesh on the gpu
		void CreateRendererResources(std::vector<MeshLoader::VertexLayout> layouts, float scale);

		// free created resources
		void DestroyResources();

	private:

		// initializes the mesh 
		void InitMesh(unsigned int index, const aiMesh* mesh, const aiScene* scene);

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Assimp::Importer mImporter = {};
		const aiScene* mScene = nullptr;

		std::vector<MeshEntry> mEntries = {};
		Dimension mDimension = {};
		uint32_t mNumVertices = 0;

		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexMemory = VK_NULL_HANDLE;
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexMemory = VK_NULL_HANDLE;
		uint32_t mIndexCount = 0;
	};	
}