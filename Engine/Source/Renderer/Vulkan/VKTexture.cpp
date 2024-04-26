#include "epch.h"
#include "VKTexture.h"

#include "VKBuffer.h"
#include "VKDevice.h"
#include "VKImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "wrapper_stb.h"

namespace Cosmos
{
	VKTexture2D::VKTexture2D(std::shared_ptr<VKDevice> device, const char* path, VkSampleCountFlagBits msaa)
		: mDevice(device), mPath(path), mMSAA(msaa)
	{
		LoadTexture();

		// image view
		mView = CreateImageView
		(
			mDevice,
			mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			mMipLevels
		);
		
		// sampler
		mSampler = CreateSampler
		(
			mDevice,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			(float)mMipLevels
		);
		
		// update descriptor
		mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mDescriptor.sampler = mSampler;
		mDescriptor.imageView = mView;
		mDescriptor.imageLayout = mLayout;
	}

	void VKTexture2D::Destroy()
	{
		vkDeviceWaitIdle(mDevice->GetDevice());
	
		vkDestroyImageView(mDevice->GetDevice(), mView, nullptr);
		vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
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
		int width = 0;
		int height = 0;

		stbi_uc* pixels = stbi_load(mPath, &width, &height, &mChannels, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to load %s texture", mPath);
			return;
		}
		
		mWidth = (uint32_t)width;
		mHeight = (uint32_t)height;
		mMipLevels = (uint32_t)(std::floor(std::log2(std::max(mWidth, mHeight)))) + 1;
		VkDeviceSize imgSize = (VkDeviceSize)(mWidth * mHeight * 4);

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
			mWidth,
			mHeight,
			mMipLevels,
			mMSAA,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mImage,
			mMemory
		);

		// transition layout to transfer data
		TransitionImageLayout
		(
			mDevice,
			Commander::Get().GetPrimary()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mMipLevels
		);

		// copy buffer to image
		{
			VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice, Commander::Get().GetPrimary()->commandPool);

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
			region.imageExtent.width = mWidth;
			region.imageExtent.height = mHeight;
			region.imageExtent.depth = 1;
			vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			EndSingleTimeCommand(mDevice, Commander::Get().GetPrimary()->commandPool, cmdBuffer);
		}

		// free staging buffer
		vkDestroyBuffer(mDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingMemory, nullptr);

		CreateMipmaps();
	}

	void VKTexture2D::CreateMipmaps()
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommand(mDevice, Commander::Get().GetPrimary()->commandPool);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = mImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = mWidth;
		int32_t mipHeight = mHeight;

		for (uint32_t i = 1; i < mMipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mMipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommand(mDevice, Commander::Get().GetPrimary()->commandPool, commandBuffer);
		int x = 0;
	}
}