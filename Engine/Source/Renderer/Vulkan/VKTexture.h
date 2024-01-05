#pragma once

#include "Renderer/Texture.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declarations
	class VKDevice;

	class VKTexture2D : public Texture2D
	{
	public:

		// creates a texture from an input file
		static std::shared_ptr<VKTexture2D> Create(std::shared_ptr<VKDevice> device, const char* path, VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT, bool ktx = false);

		// constructor
		VKTexture2D(std::shared_ptr<VKDevice>& device, const char* path, VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT, bool ktx = false);

		// destructor
		~VKTexture2D();

	public:

		// returns a reference to the image view
		virtual VkImageView GetView() override;

		// returns a reference to the image sampler
		virtual VkSampler& GetSampler() override;

	private:

		// loads the texture
		void LoadTexture();

		// creates the vulkan resources for the image
		void CreateResources();

	private:

		std::shared_ptr<VKDevice> mDevice;
		const char* mPath = nullptr;
		VkSampleCountFlagBits mMSAA = VK_SAMPLE_COUNT_1_BIT;
		bool mKTX = false;

		VkImage mImage = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
	};
}