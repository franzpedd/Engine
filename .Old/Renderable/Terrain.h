#pragma once

#include "Renderer/Device.h"
#include "Renderer/Texture.h"
#include <memory>

namespace Cosmos::renderable
{
	class Terrain
	{
	public:

		// constructor
		Terrain(std::shared_ptr<Device> device, std::string path, uint32_t width, uint32_t height, uint32_t divisions);

		// destructor
		~Terrain() = default;

	public:

		// free used resources
		void Destroy();

	private:

		// creates the heightmap texture
		void CreateTexture();

		// creates the mesh 
		void CreatePlaneMesh(uint32_t width, uint32_t height, uint32_t divisions);

		// sets up the ubo
		void PrepareUniformBuffer();

		// sets up the decriptor sets
		void SetupDescriptors();

	private:

		std::shared_ptr<Device> mDevice;
		std::string mPath;
		uint32_t mIndicesCount = 0;
		uint32_t mWidth = 0;
		uint32_t mHeight = 0;
		uint32_t mDivisions = 0;

		std::shared_ptr<Texture2D> mHeightmap;

		VkBuffer mVertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mVertexMemory = VK_NULL_HANDLE;
		VkBuffer mIndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mIndexMemory = VK_NULL_HANDLE;

		std::vector<VkBuffer> mUniformBuffer;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		// move above to renderer?
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
	};
}