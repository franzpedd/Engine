#pragma once

#include "Renderer/Vulkan/VKVertex.h"
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace Cosmos
{
	// forward declarations
	class Camera;
	class Renderer;

	class Mesh
	{
	public:

		// constructor
		Mesh(std::shared_ptr<Renderer>& renderer, std::vector<VKVertex> vertices, std::vector<uint32_t> indices);

		// destructor
		~Mesh() = default;

	public:

		// returns a reference to the vertices vector
		inline std::vector<VKVertex>& GetVerticesRef() { return mVertices; }

		// returns a reference to the indices vector
		inline std::vector<uint32_t>& GetIndicesRef() { return mIndices; }

		// returns the vertex buffer
		inline VkBuffer GetVertexBuffer() { return mVertexBuffer; }

		// returns the vertex memory
		inline VkDeviceMemory GetVertexMemory() { return mVertexMemory; }

		// returns the index buffer
		inline VkBuffer GetIndexBuffer() { return mVertexBuffer; }

		// returns the index memory
		inline VkDeviceMemory GetIndexMemory() { return mVertexMemory; }

	public:

		// draws the mesh
		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout& layout, VkDescriptorSet& descriptorSet);

		// creates the renderer resources for this mesh
		void CreateResources();

		// free used resources used by this mesh
		void DestroyResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::vector<VKVertex> mVertices;
		std::vector<uint32_t> mIndices;

		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexMemory = VK_NULL_HANDLE;
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexMemory = VK_NULL_HANDLE;
	};

	class Model
	{
	public:

		// constructor
		Model(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		~Model() = default;

	public:

		// returns the model's path
		inline std::string GetPath() const { return mPath; }

		// returns if the model is loaded
		inline bool IsLoaded() const { return mLoaded; }

	public:

		// draws the model
		void Draw(VkCommandBuffer commandBuffer);

		// updates model's logic
		void Update(float deltaTime);

		// free used resources
		void Destroy();

	public:

		// loads the model from file
		void LoadFromFile(std::string path);

	private:

		// recursively handle assimp nodes to create a tree of meshes
		void ProcessNode(aiNode* node, const aiScene* scene);

		// returns a cosmo mesh out of assimp mesh
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

		// create renderer resources
		void CreateResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;
		std::string mPath = {};
		bool mLoaded = false;
		
		std::vector<Mesh> mMeshes;
		
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;
	};
}