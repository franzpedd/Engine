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

		// constructor
		VKTexture2D(std::shared_ptr<VKDevice>& device, const char* path, VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT, bool ktx = false);
		VKTexture2D(std::shared_ptr<VKDevice>& device, tinygltf::Image& image, TextureSampler sampler, VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT);

		// destructor
		~VKTexture2D();

	public:

		// free used resoruces before destructor
		virtual void Destroy() override;

		// returns a reference to the image view
		virtual VkImageView GetView() override;

		// returns a reference to the image sampler
		virtual VkSampler& GetSampler() override;

	private:

		// loads the texture based on constructor's path
		void LoadTexture();

		// loads a texture from a tinygltf image
		void LoadTexture(tinygltf::Image& image);

		// creates mipmaps for the current bound texture
		void CreateMipmaps();

	private:

		std::shared_ptr<VKDevice> mDevice;
		const char* mPath = nullptr;
		VkSampleCountFlagBits mMSAA = VK_SAMPLE_COUNT_1_BIT;
		bool mKTX = false;

		VkImage mImage = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
		VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDescriptorImageInfo mDescriptor = {};

		int32_t mChannels = 0;
	};
}