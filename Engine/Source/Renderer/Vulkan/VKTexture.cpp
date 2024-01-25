#include "epch.h"
#include "VKTexture.h"

#include "VKBuffer.h"
#include "VKDevice.h"
#include "VKImage.h"

#include <stb_image.h>

namespace Cosmos
{
	VKTexture2D::VKTexture2D(std::shared_ptr<VKDevice>& device, const char* path, VkSampleCountFlagBits msaa, bool ktx)
		: mDevice(device), mPath(path), mMSAA(msaa), mKTX(ktx)
	{
		LOG_ASSERT(!ktx, "Khronos Texture format not yet implemented");

		LoadTexture();

		// image view
		mView = CreateImageView
		(
			mDevice,
			mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		
		// sampler
		mSampler = CreateSampler
		(
			mDevice,
			VK_FILTER_LINEAR,
			VK_FILTER_LINEAR,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT,
			VK_SAMPLER_ADDRESS_MODE_REPEAT
		);
		
		// update descriptor
		mDescriptor.sampler = mSampler;
		mDescriptor.imageView = mView;
		mDescriptor.imageLayout = mLayout;
		mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VKTexture2D::VKTexture2D(std::shared_ptr<VKDevice>& device, tinygltf::Image& image, TextureSampler sampler, VkSampleCountFlagBits msaa)
		: mDevice(device), mMSAA(msaa)
	{
		LOG_TO_TERMINAL(Logger::Warn, "Rework this to match the other constructor and use previously built functions");

		LoadTexture(image);
		CreateMipmaps();

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
			(VkFilter)sampler.min,
			(VkFilter)sampler.mag,
			(VkSamplerAddressMode)sampler.u,
			(VkSamplerAddressMode)sampler.v,
			(VkSamplerAddressMode)sampler.w,
			(float)mMipLevels
		);

		// update descriptor
		mDescriptor.sampler = mSampler;
		mDescriptor.imageView = mView;
		mDescriptor.imageLayout = mLayout;
		mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VKTexture2D::~VKTexture2D()
	{
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
		vkDestroyImageView(mDevice->GetDevice(), mView, nullptr);
		vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mMemory, nullptr);
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
		stbi_uc* pixels = stbi_load(mPath, &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to load %s texture", mPath);
			return;
		}

		mMipLevels = (int32_t)(std::floor(std::log2(std::max(mWidth, mHeight))) + 1.0);
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
			region.imageExtent.width = mWidth;
			region.imageExtent.height = mHeight;
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

	void VKTexture2D::LoadTexture(tinygltf::Image& image)
	{
		unsigned char* buffer = nullptr;
		VkDeviceSize bufferSize = 0;
		bool deleteBuffer = false;

		if (image.component == 3) // covert RGB to RGBA to normalize loading 
		{
			bufferSize = (VkDeviceSize)image.width * image.height * 4;
			buffer = new unsigned char[bufferSize];

			unsigned char* rgba = buffer;
			unsigned char* rgb = &image.image[0];

			for (int32_t i = 0; i < image.width * image.height; i++)
			{
				for (int32_t j = 0; j < 3; j++)
				{
					rgba[j] = rgb[j];
				}

				rgba += 4;
				rgb += 3;
			}

			deleteBuffer = true;
		}

		else
		{
			buffer = &image.image[0];
			bufferSize = image.image.size();
		}

		mWidth = image.width;
		mHeight = image.height;
		mMipLevels = (int32_t)(std::floor(std::log2(std::max(mWidth, mHeight))) + 1.0);

		// create staging buffer for image
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		if (buffer == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Error, "Buffer is not initialized");
			return;
		}
		
		BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			bufferSize,
			&stagingBuffer,
			&stagingMemory
		);

		void* data = nullptr;
		vkMapMemory(mDevice->GetDevice(), stagingMemory, 0, bufferSize, 0, &data);
		memcpy(data, buffer, (size_t)bufferSize);
		vkUnmapMemory(mDevice->GetDevice(), stagingMemory);

		if(deleteBuffer)
			delete[] buffer;

		VkFormatProperties properties = {};
		vkGetPhysicalDeviceFormatProperties(mDevice->GetPhysicalDevice(), VK_FORMAT_R8G8B8A8_SRGB, &properties);
		LOG_ASSERT(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT, "Physical device doesnt support blit from source");
		LOG_ASSERT(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT, "Physical device doesnt support blit from destiny");

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
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // also VK_IMAGE_USAGE_TRANSFER_SRC_BIT ?
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mImage,
			mMemory
		);

		// record the command buffer
		VkCommandBuffer cmd = CreateCommandBuffer
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
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
		vkCmdCopyBufferToImage(cmd, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// transition image layout to src optimal
		TransitionImageLayout
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		// send the command buffer to be processed
		FlushCommandBuffer
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			cmd,
			mDevice->GetGraphicsQueue(),
			true
		);

		// free staging buffer
		vkDestroyBuffer(mDevice->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingMemory, nullptr);
	}

	void VKTexture2D::CreateMipmaps()
	{
		VkCommandBuffer cmd = CreateCommandBuffer
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
		);

		for (int32_t i = 0; i < mMipLevels; i++)
		{
			VkImageBlit imageBlit = {};
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(mWidth >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(mHeight >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;
			
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(mWidth >> i);
			imageBlit.dstOffsets[1].y = int32_t(mHeight >> i);
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			// insert memory barrier to transfer dst
			InsertImageMemoryBarrier
			(
				cmd,
				mImage,
				VK_ACCESS_NONE,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange
			);

			// blit the image
			vkCmdBlitImage
			(
				cmd,
				mImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				VK_FILTER_LINEAR
			);

			// insert memory barrier to set the image to read only
			InsertImageMemoryBarrier
			(
				cmd,
				mImage,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange
			);
		}

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = mMipLevels;

		// insert memory barrier to read only optimal
		InsertImageMemoryBarrier
		(
			cmd,
			mImage,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			subresourceRange
		);

		// finish the command buffer
		FlushCommandBuffer
		(
			mDevice,
			mDevice->GetMainCommandEntry()->commandPool,
			cmd,
			mDevice->GetGraphicsQueue(),
			true
		);
	}
}