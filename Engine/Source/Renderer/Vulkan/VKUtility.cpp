#include "VKUtility.h"
#include "VKDevice.h"
#include "Util/Logger.h"

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
		VK_ASSERT(vkCreateImage(device->Device(), &imageCI, nullptr, &image), "Failed to create image");

		// alocate memory for specified image
		VkMemoryRequirements memoryReqs = {};
		vkGetImageMemoryRequirements(device->Device(), image, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = device->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(device->Device(), &memoryAllocInfo, nullptr, &memory), "Failed to allocate memory for image");

		// link image with allocated memory
		vkBindImageMemory(device->Device(), image, memory, 0);
	}

	void TransitionImageLayout(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
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
		barrier.subresourceRange.levelCount = 1;
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

	VkImageView CreateImageView(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.image = image;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = format;
		imageViewCI.subresourceRange.aspectMask = aspectFlags;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_ASSERT(vkCreateImageView(device->Device(), &imageViewCI, nullptr, &imageView), "Failed to create image view");

		return imageView;
	}

	VkResult BufferCreate(std::shared_ptr<VKDevice>& device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// specify buffer
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT(vkCreateBuffer(device->Device(), &bufferCI, nullptr, buffer), "Failed to create buffer");

		// alocate memory for specified buffer 
		VkMemoryRequirements memoryReqs;
		vkGetBufferMemoryRequirements(device->Device(), *buffer, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = device->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(device->Device(), &memoryAllocInfo, nullptr, memory), "Failed to allocate memory for buffer");

		// data is not null, must map the buffer and copy the data
		if (data != nullptr)
		{
			void* mapped;
			VK_ASSERT(vkMapMemory(device->Device(), *memory, 0, size, 0, &mapped), "Faled to map memory");
			memcpy(mapped, data, size);

			// if host coherency hasn't been requested, do a manual flush to make writes visible
			if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = {};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(device->Device(), 1, &mappedRange);
			}

			vkUnmapMemory(device->Device(), *memory);
		}

		// link buffer with allocated memory
		VK_ASSERT(vkBindBufferMemory(device->Device(), *buffer, *memory, 0), "Failed to bind buffer with memory");

		return VK_SUCCESS;
	}

	void BufferCopy(std::shared_ptr<VKDevice>& device, VkBuffer src, VkBuffer dst, VkDeviceSize size, VkCommandPool& commandPool)
	{
		// begin command buffer
		VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
		cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocateInfo.pNext = nullptr;
		cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocateInfo.commandPool = commandPool;
		cmdBufferAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocateInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		// copy the region
		VkBufferCopy region = {};
		region.size = size;
		vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &region);

		// end command buffer
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->GraphicsQueue());

		vkFreeCommandBuffers(device->Device(), commandPool, 1, &cmdBuffer);
	}

	void BufferCopyToImage(std::shared_ptr<VKDevice>& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool)
	{
		// begin command buffer
		VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
		cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocateInfo.pNext = nullptr;
		cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocateInfo.commandPool = commandPool;
		cmdBufferAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocateInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		// copy the buffer to the image
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
		vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// end command buffer
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->GraphicsQueue());

		vkFreeCommandBuffers(device->Device(), commandPool, 1, &cmdBuffer);
	}

	VkFormat FindSuitableFormat(std::shared_ptr<VKDevice>& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->PhysicalDevice(), format, &props);

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

	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandPool = commandPool;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocInfo, &commandBuffer);

		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->GraphicsQueue());

		vkFreeCommandBuffers(device->Device(), commandPool, 1, &commandBuffer);
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

	VkCommandBuffer CreateCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandBufferCount = 1;
		cmdBufferAllocInfo.commandPool = cmdPool;
		cmdBufferAllocInfo.level = level;

		VkCommandBuffer cmdBuffer;
		VK_ASSERT(vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocInfo, &cmdBuffer), "Failed to allocate command buffers");

		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufferBI = {};
			cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBI.pNext = nullptr;
			cmdBufferBI.flags = 0;
			VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI), "Failed to initialize command buffer");
		}

		return cmdBuffer;
	}

	void BeginCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferBeginInfo cmdBufferBI = {};
		cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBI.pNext = nullptr;
		cmdBufferBI.flags = 0;
		VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI), "Failed to initialize command buffer");
	}

	void FlushCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, bool free)
	{
		VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end the recording of the command buffer");

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkFenceCreateInfo fenceCI = {};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = 0;

		VkFence fence;
		VK_ASSERT(vkCreateFence(device->Device(), &fenceCI, nullptr, &fence), "Failed to create fence for command buffer submission");
		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence), "Failed to submit command buffer");
		VK_ASSERT(vkWaitForFences(device->Device(), 1, &fence, VK_TRUE, 100000000000), "Failed to wait for fences");

		vkDestroyFence(device->Device(), fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(device->Device(), cmdPool, 1, &cmdBuffer);
		}
	}
}