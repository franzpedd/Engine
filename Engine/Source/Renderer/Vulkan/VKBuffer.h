#pragma once

#include "Core/Defines.h"
#include "VKDevice.h"

#include <vulkan/vulkan.h>

namespace Cosmos
{
	class COSMOS_API VKBuffer
	{
	public:

		// constructor
		VKBuffer(std::shared_ptr<VKDevice>& device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, bool map = true);
		
		// destructor
		~VKBuffer();

		// mappes the used memory
		void Map();

		// unmaps the used memory
		void Unmap();

		// flushes the memory
		void Flush(VkDeviceSize size = VK_WHOLE_SIZE);

	public:

		std::shared_ptr<VKDevice>& m_Device;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_DescriptorBufferInfo = {};
		i32 m_Count = 0;
		void* m_Mapped = nullptr;
	};
}