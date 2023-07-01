#include "VKTexture.h"

#include <gli/gli.hpp>

#include <cmath>

namespace Cosmos
{
	VKTexture2D::VKTexture2D(std::shared_ptr<VKDevice>& device)
		: m_Device(device)
	{
	}

	VKTexture2D::~VKTexture2D()
	{
		vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.view, nullptr);
		vkDestroyImage(m_Device->GetSpecification().Device, m_Specification.image, nullptr);

		if (m_Specification.sampler != nullptr)
		{
			vkDestroySampler(m_Device->GetSpecification().Device, m_Specification.sampler, nullptr);
		}

		vkFreeMemory(m_Device->GetSpecification().Device, m_Specification.memory, nullptr);
	}

	void VKTexture2D::LoadFromFile(std::string path, VkFormat format, VkQueue queue, VkImageUsageFlags usage, VkImageLayout layout)
	{
		gli::texture2d tex2D(gli::load(path.c_str()));
		LOG_ASSERT(!tex2D.empty(), "Failed to load texture");

		m_Specification.width = (u32)tex2D[0].extent().x;
		m_Specification.height = (u32)tex2D[0].extent().y;
		m_Specification.mipLevel = (u32)tex2D.levels();

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_Device->GetSpecification().PhysicalDevice, format, &formatProperties);
		
		VkCommandBuffer commandBuffer = m_Device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = tex2D.size();
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(m_Device->GetSpecification().Device, &bufferCreateInfo, nullptr, &stagingBuffer));

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(m_Device->GetSpecification().Device, stagingBuffer, &memReqs);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.pNext = nullptr;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = m_Device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memAllocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_Device->GetSpecification().Device, stagingBuffer, stagingMemory, 0));

		uint8_t* data = nullptr;
		VK_CHECK_RESULT(vkMapMemory(m_Device->GetSpecification().Device, stagingMemory, 0, memReqs.size, 0, (void**)&data));
		memcpy(data, tex2D.data(), tex2D.size());
		vkUnmapMemory(m_Device->GetSpecification().Device, stagingMemory);

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_Specification.mipLevel; i++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);
			offset += static_cast<uint32_t>(tex2D[i].size());
		}

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = m_Specification.mipLevel;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { m_Specification.width, m_Specification.height, 1 };
		imageCreateInfo.usage = usage;
		if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)){ imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
		VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCreateInfo, nullptr, &m_Specification.image));

		vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = m_Device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memAllocInfo, nullptr, &m_Specification.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.image, m_Specification.memory, 0));

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_Specification.mipLevel;
		subresourceRange.layerCount = 1;

		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_Specification.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (u32)bufferCopyRegions.size(),bufferCopyRegions.data());

		m_Specification.layout = layout;

		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = m_Specification.layout;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		m_Device->FlushCommandBuffer(commandBuffer, queue);

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = (f32)m_Specification.mipLevel;
		samplerCreateInfo.maxAnisotropy = m_Device->GetSpecification().EnabledFeatures.samplerAnisotropy ? m_Device->GetSpecification().Properties.limits.maxSamplerAnisotropy : 1.0f;
		samplerCreateInfo.anisotropyEnable = m_Device->GetSpecification().EnabledFeatures.samplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(m_Device->GetSpecification().Device, &samplerCreateInfo, nullptr, &m_Specification.sampler));

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.levelCount = m_Specification.mipLevel;
		viewCreateInfo.image = m_Specification.image;
		VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &viewCreateInfo, nullptr, &m_Specification.view));

		// used by descriptor sets
		m_Specification.descriptorImageInfo.sampler = m_Specification.sampler;
		m_Specification.descriptorImageInfo.imageLayout = m_Specification.layout;
		m_Specification.descriptorImageInfo.imageView = m_Specification.view;

		// cleanup staging resources
		vkFreeMemory(m_Device->GetSpecification().Device, stagingMemory, nullptr);
		vkDestroyBuffer(m_Device->GetSpecification().Device, stagingBuffer, nullptr);
	}

	void VKTexture2D::LoadFromBuffer(void* buffer, VkDeviceSize size, VkFormat format, u32 width, u32 height, VkQueue queue, VkFilter filter, VkImageUsageFlags usage, VkImageLayout layout)
	{
		m_Specification.width = width;
		m_Specification.height = height;
		m_Specification.mipLevel = 1;

		VkCommandBuffer commandBuffer = m_Device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.pNext = nullptr;
		bufferCI.size = size;
		bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(m_Device->GetSpecification().Device, &bufferCI, nullptr, &stagingBuffer));
		
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_Device->GetSpecification().Device, stagingBuffer, &memoryRequirements);

		VkMemoryAllocateInfo memoryAI = {};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.pNext = nullptr;
		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_Device->GetSpecification().Device, stagingBuffer, stagingMemory, 0));

		u8* data = nullptr;
		VK_CHECK_RESULT(vkMapMemory(m_Device->GetSpecification().Device, stagingMemory, 0, memoryRequirements.size, 0, (void**)&data));
		memcpy(data, buffer, (size_t)size);
		vkUnmapMemory(m_Device->GetSpecification().Device, stagingMemory);

		VkBufferImageCopy bufferImageCopy = {};
		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.baseArrayLayer = 0;
		bufferImageCopy.imageSubresource.layerCount = 1;
		bufferImageCopy.imageExtent.width = m_Specification.width;
		bufferImageCopy.imageExtent.height = m_Specification.height;
		bufferImageCopy.imageExtent.depth = 1;
		bufferImageCopy.bufferOffset = 0;

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.pNext = nullptr;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = format;
		imageCI.mipLevels = m_Specification.mipLevel;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.extent.width = m_Specification.width;
		imageCI.extent.height = m_Specification.height;
		imageCI.extent.depth = 1;
		imageCI.usage = usage;

		if (!(imageCI.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCI.usage &= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCI, nullptr, &m_Specification.image));

		vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.image, &memoryRequirements);

		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &m_Specification.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.image, m_Specification.memory, 0));

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_Specification.mipLevel;
		subresourceRange.layerCount = 1;
		
		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_Specification.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
		
		m_Specification.layout = layout;

		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = m_Specification.layout;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		m_Device->FlushCommandBuffer(commandBuffer, queue);

		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = filter;
		samplerCI.minFilter = filter;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		VK_CHECK_RESULT(vkCreateSampler(m_Device->GetSpecification().Device, &samplerCI, nullptr, &m_Specification.sampler));

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.pNext = nullptr;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = format;
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.image = m_Specification.image;
		VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.view));

		m_Specification.descriptorImageInfo.sampler = m_Specification.sampler;
		m_Specification.descriptorImageInfo.imageLayout = m_Specification.layout;
		m_Specification.descriptorImageInfo.imageView = m_Specification.view;

		vkFreeMemory(m_Device->GetSpecification().Device, stagingMemory, nullptr);
		vkDestroyBuffer(m_Device->GetSpecification().Device, stagingBuffer, nullptr);
	}

	VKTextureCubemap::VKTextureCubemap(std::shared_ptr<VKDevice>& device)
		: m_Device(device)
	{
	}

	VKTextureCubemap::~VKTextureCubemap()
	{
		vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.view, nullptr);
		vkDestroyImage(m_Device->GetSpecification().Device, m_Specification.image, nullptr);

		if (m_Specification.sampler != nullptr)
		{
			vkDestroySampler(m_Device->GetSpecification().Device, m_Specification.sampler, nullptr);
		}

		vkFreeMemory(m_Device->GetSpecification().Device, m_Specification.memory, nullptr);
	}

	void VKTextureCubemap::LoadFromFile(std::string path, VkFormat format, VkQueue queue, VkImageUsageFlags usage, VkImageLayout layout)
	{
		gli::texture_cube texCube(gli::load(path));
		LOG_ASSERT(!texCube.empty(), "Failed to load texture");

		m_Specification.width = (i32)texCube.extent().x;
		m_Specification.height = (i32)texCube.extent().y;
		m_Specification.mipLevel = (i32)texCube.levels();

		VkCommandBuffer commandBuffer = m_Device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.pNext = nullptr;
		bufferCI.size = texCube.size();
		bufferCI.usage = usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(m_Device->GetSpecification().Device, &bufferCI, nullptr, &stagingBuffer));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_Device->GetSpecification().Device, stagingBuffer, &memoryRequirements);

		VkMemoryAllocateInfo memoryAI = {};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.pNext = nullptr;
		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_Device->GetSpecification().Device, stagingBuffer, stagingMemory, 0));

		u8* data = nullptr;
		VK_CHECK_RESULT(vkMapMemory(m_Device->GetSpecification().Device, stagingMemory, 0, memoryRequirements.size, 0, (void**)&data));
		memcpy(data, texCube.data(), texCube.size());
		vkUnmapMemory(m_Device->GetSpecification().Device, stagingMemory);

		std::vector<VkBufferImageCopy> bufferCopyRegions = {};
		size_t offset = 0;

		LOG_WARNING("Cubemap sizes may be incorrect, check this at a later stage");
		for (u32 face = 0; face < 6; face++)
		{
			for (u32 level = 0; level < m_Specification.mipLevel; level++)
			{
				VkBufferImageCopy bufferImageCopy = {};
				bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferImageCopy.imageSubresource.mipLevel = level;
				bufferImageCopy.imageSubresource.baseArrayLayer = face;
				bufferImageCopy.imageSubresource.layerCount = 1;
				bufferImageCopy.imageExtent.width = (u32)texCube[face][level].extent().x;
				bufferImageCopy.imageExtent.height = (u32)texCube[face][level].extent().y;
				bufferImageCopy.imageExtent.depth = 1;
				bufferImageCopy.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferImageCopy);

				offset += texCube.size();
			}
		}

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.pNext = nullptr;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = format;
		imageCI.mipLevels = m_Specification.mipLevel;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.extent.width = m_Specification.width;
		imageCI.extent.height = m_Specification.height;
		imageCI.extent.depth = 1;
		imageCI.usage = usage;
		
		if (!(imageCI.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCI.usage &= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		imageCI.arrayLayers = 6; // cube faces counts as layers in vulkan
		imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCI, nullptr, &m_Specification.image));

		vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.image, &memoryRequirements);

		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &m_Specification.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.image, m_Specification.memory, 0));

		VkImageSubresourceRange imageSubresourceRange = {};
		imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceRange.baseMipLevel = 0;
		imageSubresourceRange.levelCount = m_Specification.mipLevel;
		imageSubresourceRange.layerCount = 0;

		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = imageSubresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_Specification.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (u32)bufferCopyRegions.size(), bufferCopyRegions.data());
		
		m_Specification.layout = layout;

		{
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = m_Specification.layout;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.image = m_Specification.image;
			imageMemoryBarrier.subresourceRange = imageSubresourceRange;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		m_Device->FlushCommandBuffer(commandBuffer, queue);

		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.maxAnisotropy = m_Device->GetSpecification().EnabledFeatures.samplerAnisotropy ? m_Device->GetSpecification().Properties.limits.maxSamplerAnisotropy : 1.0f;
		samplerCI.anisotropyEnable = m_Device->GetSpecification().EnabledFeatures.samplerAnisotropy;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = (f32)m_Specification.mipLevel;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(m_Device->GetSpecification().Device, &samplerCI, nullptr, &m_Specification.sampler));

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.pNext = nullptr;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageViewCI.format = format;
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.image = m_Specification.image;
		VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.view));

		m_Specification.descriptorImageInfo.sampler = m_Specification.sampler;
		m_Specification.descriptorImageInfo.imageLayout = m_Specification.layout;
		m_Specification.descriptorImageInfo.imageView = m_Specification.view;

		vkFreeMemory(m_Device->GetSpecification().Device, stagingMemory, nullptr);
		vkDestroyBuffer(m_Device->GetSpecification().Device, stagingBuffer, nullptr);
	}
}