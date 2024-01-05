#include "epch.h"
#include "VKTexture.h"

#include "VKBuffer.h"
#include "VKDevice.h"
#include "VKImage.h"

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26819 26451 26495 6262)
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Cosmos
{
	std::shared_ptr<VKTexture2D> VKTexture2D::Create(std::shared_ptr<VKDevice> device, const char* path, VkSampleCountFlagBits msaa, bool ktx)
	{
		return std::make_shared<VKTexture2D>(device, path, msaa, ktx);
	}

	VKTexture2D::VKTexture2D(std::shared_ptr<VKDevice>& device, const char* path, VkSampleCountFlagBits msaa, bool ktx)
		: mDevice(device), mPath(path), mMSAA(msaa), mKTX(ktx)
	{
		LOG_ASSERT(!ktx, "Khronos Texture format not yet implemented");

		LoadTexture();
		CreateResources();
	}

	VKTexture2D::~VKTexture2D()
	{
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
		vkDestroyImageView(mDevice->GetDevice(), mView, nullptr);
		vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
	}

	VkImageView VKTexture2D::GetView()
	{
		return mView;
	}

	VkSampler& VKTexture2D::GetSampler()
	{
		return mSampler;
	}

	void VKTexture2D::LoadTexture()
	{
		int width;
		int height;
		int channels;

		stbi_uc* pixels = stbi_load(mPath, &width, &height, &channels, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to load %s texture", mPath);
			return;
		}

		VkDeviceSize imgSize = (VkDeviceSize)(width * height * 4);

		// create staging buffer for image
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			imgSize,
			&stagingBuffer,
			&stagingMemory
		);

		void* data = nullptr;
		vkMapMemory(mDevice->GetDevice(), stagingMemory, 0, imgSize, 0, &data);
		memcpy(data, pixels, (size_t)imgSize);
		vkUnmapMemory(mDevice->GetDevice(), stagingMemory);

		stbi_image_free(pixels);

		// create image resource
		CreateImage
		(
			mDevice,
			width,
			height,
			1,
			mMSAA,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mImage,
			mMemory
		);

		// transition layout to transfer data
		TransitionImageLayout
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// copy buffer to image
		{
			VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice, mDevice->GetMainCommandEntry()->commandPool);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset.x = 0;
			region.imageOffset.y = 0;
			region.imageOffset.z = 0;
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			EndSingleTimeCommand(mDevice, mDevice->GetMainCommandEntry()->commandPool, cmdBuffer);
		}

		// transition image layout to shader optimal
		TransitionImageLayout
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);

		// free staging buffer
		vkDestroyBuffer(mDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingMemory, nullptr);
	}

	void VKTexture2D::CreateResources()
	{
		// image view
		mView = CreateImageView
		(
			mDevice,
			mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		// sampler
		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.anisotropyEnable = VK_TRUE;
		samplerCI.maxAnisotropy = mDevice->GetProperties().limits.maxSamplerAnisotropy; //properties.limits.maxSamplerAnisotropy;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = VK_FALSE;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VK_ASSERT(vkCreateSampler(mDevice->GetDevice(), &samplerCI, nullptr, &mSampler), "Failed to create sampler");
	}
}