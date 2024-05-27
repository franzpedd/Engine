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
		~Skybox() = default;

		// returns the paths array reference
		inline std::array<std::string, 6> GetPathsRef() { return mPaths; }

	public:

		// loads the skybox
		void LoadSkybox();

		// updates skybox logic
		void OnUpdate(float deltaTime, glm::mat4 transform);

		// renders the skybox
		void OnRender(VkCommandBuffer commandBuffer);

		// free used resources
		void Destroy();

	private:

		// creates used resources for the skybox
		void CreateResources();

		// updates the descriptor set (used when properties has changed)
		void UpdateDescriptorSets();

	private:

		Shared<Renderer> mRenderer;
		Shared<Camera> mCamera;
		Shared<TextureCubemap> mCubemap;
		std::array<std::string, 6> mPaths = {};

		Shared<Model> mSkyboxModel;
	
		// camera's ubo
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		// skybox descriptor
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = {};

		// skybox model
		std::vector<glm::vec3> mVertices = {};
		std::vector<uint32_t> mIndices = {};
		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexMemory = VK_NULL_HANDLE;
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexMemory = VK_NULL_HANDLE;
	};
}