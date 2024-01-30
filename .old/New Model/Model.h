#pragma once

#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Util/Math.h"
#include "Util/MeshLoader.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	class Renderer;
	class Camera;

	class Model
	{
	public:

		// returns a smart-ptr to a new model
		static std::shared_ptr<Model> Create(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// constructor
		Model(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		~Model();

	public:

		// returns the model's path
		inline std::string GetPath() { return mPath; }

		// returns if model was successufly loaded
		inline bool IsLoaded() const { return mLoaded; }

	public:

		// updates the model's logic
		void Update(float deltaTime);

		// draws the model
		void Draw(VkCommandBuffer commandBuffer);

		// loads a new model given it's path
		void LoadFromFile(std::string path);

		// cleans the used resources
		void Destroy();

	private:

		// create renderer resources
		void CreateResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;
		std::string mPath = {};
		bool mLoaded = false;

		// eventually will be automatically created by the renderer
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = {};
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

		// ubo
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		// mesh
		std::unique_ptr<MeshLoader> mMesh;
	};
}