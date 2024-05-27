#include "epch.h"
#include "VKTexture.h"

#include "VKBuffer.h"
#include "VKCommander.h"
#include "VKDevice.h"
#include "VKImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "wrapper_stb.h"

namespace Cosmos
{
	VKTexture2D::VKTexture2D(Shared<VKDevice> device, const char* path, VkSampleCountFlagBits msaa)
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
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			(float)mMipLevels
		);
	}

	VKTexture2D::~VKTexture2D()
	{
		vkDeviceWaitIdle(mDevice->GetDevice());

		vkDestroyImageView(mDevice->GetDevice(), mView, nullptr);
		vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
	}

	void VKTexture2D::LoadTexture()
	{
		stbi_uc* pixels = stbi_load(mPath, &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to load %s texture", mPath);
			return;
		}
		
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
			1,
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
			VKCommander::GetInstance()->GetMainRef()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mMipLevels
		);

		// copy buffer to image
		{
			VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool);

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

			EndSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool, cmdBuffer);
		}

		// free staging buffer
		vkDestroyBuffer(mDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingMemory, nullptr);

		CreateMipmaps();
	}

	void VKTexture2D::CreateMipmaps()
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool);

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

		EndSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool, commandBuffer);
		int x = 0;
	}

	VKTextureCubemap::VKTextureCubemap(Shared<VKDevice> device, std::array<std::string, 6> paths, VkSampleCountFlagBits msaa)
		: mDevice(device), mPaths(paths), mMSAA(msaa)
	{
		LoadTextures();

		// image view
		mView = CreateImageView
		(
			mDevice,
			mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			mMipLevels,
			6,
			VK_IMAGE_VIEW_TYPE_CUBE
		);

		// sampler
		mSampler = CreateSampler
		(
			mDevice,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			(float)mMipLevels
		);
	}

	VKTextureCubemap::~VKTextureCubemap()
	{
		vkDeviceWaitIdle(mDevice->GetDevice());

		vkDestroyImageView(mDevice->GetDevice(), mView, nullptr);
		vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
	}

	void VKTextureCubemap::LoadTextures()
	{
		LOG_ASSERT(mPaths.size() == 6, "A Skybox must have 6 textures but currently have %d", mPaths.size());

		VkDeviceSize imageSize;
		VkDeviceSize layerSize;
		uint8_t *data;
        uint64_t memAddress;
        stbi_uc* pixels;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		
		for(int8_t i = 0; i < 6; i++)
		{
			stbi_uc* pixels = stbi_load(mPaths[i].c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);

			if(!pixels)
			{
				LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to load %s texture", mPaths[i].c_str());
				return;
			}

			if (i == 0)
            {
                layerSize = mWidth * mHeight * mChannels;
                imageSize = layerSize * 6;

                BufferCreate
				(
					mDevice,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					imageSize,
					&stagingBuffer,
					&stagingMemory
				);

                vkMapMemory(mDevice->GetDevice(), stagingMemory, 0, imageSize, 0, (void **)&data);
                memAddress = reinterpret_cast<uint64_t>(data);
            }

			memcpy(reinterpret_cast<uint8_t*>(memAddress), static_cast<void*>(pixels), static_cast<size_t>(layerSize));
            stbi_image_free(pixels);
            memAddress += layerSize;
		}

		vkUnmapMemory(mDevice->GetDevice(), stagingMemory);

		// create image resource
		CreateImage
		(
			mDevice,
			mWidth,
			mHeight,
			mMipLevels,
			6,
			mMSAA,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mImage,
			mMemory,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		);

		// Setup buffer copy regions for each face including all of its miplevels
		std::vector<VkBufferImageCopy> regions = {};
		size_t offset = 0;
		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t level = 0; level < mMipLevels; level++)
			{
				// calculate offset into staging buffer for the current mip level and face
				offset = mWidth * mHeight * mChannels * face * sizeof(VkDeviceSize);

				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = mWidth;
				bufferCopyRegion.imageExtent.height = mHeight;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;
				bufferCopyRegion.bufferRowLength = 0;
				bufferCopyRegion.bufferImageHeight = 0;
				regions.push_back(bufferCopyRegion);
			}
		}

		// transition layout to transfer data
		TransitionImageLayout
		(
			mDevice,
			VKCommander::GetInstance()->GetMainRef()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mMipLevels,
			6
		);

		// copy buffer to image
		{
			VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool);

			vkCmdCopyBufferToImage
			(
				cmdBuffer, 
				stagingBuffer, 
				mImage, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				(uint32_t)regions.size(),
				regions.data()
			);

			EndSingleTimeCommand(mDevice, VKCommander::GetInstance()->GetMainRef()->commandPool, cmdBuffer);
		}

		// transition layout to transfer data
		TransitionImageLayout
		(
			mDevice,
			VKCommander::GetInstance()->GetMainRef()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mMipLevels,
			6
		);

		// free staging buffer
		vkDestroyBuffer(mDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingMemory, nullptr);
	}
}