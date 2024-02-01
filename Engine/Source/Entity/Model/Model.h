#pragma once

#include "Entity/Model/Mesh.h"
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
	class Texture2D;

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

		// returns the model's albedo
		inline std::string GetAlbedoPath() const { return mAlbedoPath; }

		// returns if the model is loaded
		inline bool IsLoaded() const { return mLoaded; }

	public:

		// draws the model
		void Draw(VkCommandBuffer commandBuffer);

		// updates model's logic
		void Update(float deltaTime, glm::mat4 transform);

		// free used resources
		void Destroy();

	public:

		// loads the model from file
		void LoadFromFile(std::string path);

		// loads the model albedo texture
		void LoadAlbedoTexture(std::string path);

	public:

		// translates the model given an orientation delta
		void Translate(glm::vec3 delta);

		// rotates the model given up-to-3 orientation
		void Rotate(float radians, glm::vec3 orientationVec);

		// scales the model given it's multiplication factor
		void Scale(glm::vec3 factor);

	private:

		// recursively handle assimp nodes to create a tree of meshes
		void ProcessNode(aiNode* node, const aiScene* scene);

		// returns a cosmo mesh out of assimp mesh
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

		// create renderer resources
		void CreateResources();

		// updates the descriptor set (used when properties has changed)
		void UpdateDescriptorSets(); 

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;
		std::string mPath = {};
		bool mLoaded = false;
		
		std::vector<Mesh> mMeshes;
		
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
		
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		std::string mAlbedoPath = "Data/Textures/dev/colors/orange.png";
		std::shared_ptr<Texture2D> mAlbedoTexture;
		bool mLoadedAlbedo = false;
	};
}