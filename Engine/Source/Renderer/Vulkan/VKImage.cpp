#include "epch.h"
#include "VKImage.h"

#include "VKBuffer.h"
#include "VKDevice.h"

namespace Cosmos
{
	void CreateImage(std::shared_ptr<VKDevice>& device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory)
	{
		// specify image
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.pNext = nullptr;
		imageCI.flags = 0;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.extent.width = width;
		imageCI.extent.height = height;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = mipLevels;
		imageCI.arrayLayers = 1;
		imageCI.format = format;
		imageCI.tiling = tiling;
		imageCI.usage = usage;
		imageCI.samples = samples;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT(vkCreateImage(device->GetDevice(), &imageCI, nullptr, &image), "Failed to create image");

		// alocate memory for specified image
		VkMemoryRequirements memoryReqs = {};
		vkGetImageMemoryRequirements(device->GetDevice(), image, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = device->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(device->GetDevice(), &memoryAllocInfo, nullptr, &memory), "Failed to allocate memory for image");

		// link image with allocated memory
		vkBindImageMemory(device->GetDevice(), image, memory, 0);
	}

	VkSampler CreateSampler(std::shared_ptr<VKDevice>& device, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels)
	{
		VkSampler sampler = VK_NULL_HANDLE;

		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = mag;
		samplerCI.minFilter = min;
		samplerCI.addressModeU = u;
		samplerCI.addressModeV = v;
		samplerCI.addressModeW = w;
		samplerCI.anisotropyEnable = VK_TRUE;
		samplerCI.maxAnisotropy = device->GetProperties().limits.maxSamplerAnisotropy;
		samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = VK_FALSE;
		samplerCI.maxLod = (float)mipLevels;
		samplerCI.minLod = 0.0f;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VK_ASSERT(vkCreateSampler(device->GetDevice(), &samplerCI, nullptr, &sampler), "Failed to create sampler");

		return sampler;
	}

	void TransitionImageLayout(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(device, cmdPool);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		else
		{
			LOG_ASSERT(false, "Invalid layout transition");
		}

		vkCmdPipelineBarrier
		(
			cmdBuffer,
			sourceStage,
			destinationStage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		EndSingleTimeCommand(device, cmdPool, cmdBuffer);
	}

	VkImageView CreateImageView(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevel)
	{
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = format;
		imageViewCI.subresourceRange.aspectMask = aspectFlags;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = mipLevel;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_ASSERT(vkCreateImageView(device->GetDevice(), &imageViewCI, nullptr, &imageView), "Failed to create image view");

		return imageView;
	}

	void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = 0;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier
		(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier
		);
	}

	VkFormat FindSuitableFormat(std::shared_ptr<VKDevice>& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}

			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		LOG_ASSERT(false, "Failed to find suitable format");
	}

	VkFormat FindDepthFormat(std::shared_ptr<VKDevice>& device)
	{
		const std::vector<VkFormat>& candidates =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		return FindSuitableFormat(device, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}