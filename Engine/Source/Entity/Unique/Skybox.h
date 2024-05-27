#pragma once

#include "Core/Camera.h"
#include "Entity/Renderable/Model.h"
#include "Renderer/Texture.h"
#include "Renderer/Renderer.h"

namespace Cosmos
{
	class Skybox
	{
	public:

		// constructor
		Skybox(Shared<Renderer> renderer, Shared<Camera> camera);

		// destructor
		~Skybox();

		// returns if the skybox was loaded
		inline bool IsLoaded() { return mLoaded; }

		// returns the paths array reference
		inline std::array<std::string, 6> GetPathsRef() { return mPaths; }

	public:

		// loads the skybox
		void LoadSkybox();

		// updates skybox logic
		void OnUpdate(float timestep);

		// renders the skybox
		void OnRender(VkCommandBuffer commandBuffer);

	private:

		// creates used resources for the skybox
		void CreateResources();

		// updates the descriptor set (used when properties has changed)
		void UpdateDescriptorSets();

	private:

		Shared<Renderer> mRenderer;
		Shared<Camera> mCamera;
		Shared<TextureCubemap> mCubemap;
		Shared<Model> mCubemodel;
		bool mLoaded = false;
		std::array<std::string, 6> mPaths = {};
	
		// camera's ubo
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		// skybox descriptor
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = {};
	};
}