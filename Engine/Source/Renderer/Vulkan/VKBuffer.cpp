#include "VKBuffer.h"

namespace Cosmos
{
	VKBuffer::VKBuffer(std::shared_ptr<VKDevice>& device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, bool map)
		: m_Device(device)
	{
		m_Device->CreateBuffer(usage, properties, size, &m_Buffer, &m_Memory);
		m_DescriptorBufferInfo.buffer = m_Buffer;
		m_DescriptorBufferInfo.offset = 0;
		m_DescriptorBufferInfo.range = size;

		if (map)
		{
			VK_CHECK_RESULT(vkMapMemory(m_Device->GetSpecification().Device, m_Memory, 0, size, 0, &m_Mapped));
		}
	}

	VKBuffer::~VKBuffer()
	{
		Unmap();
		vkDestroyBuffer(m_Device->GetSpecification().Device, m_Buffer, nullptr);
		vkFreeMemory(m_Device->GetSpecification().Device, m_Memory, nullptr);
		m_Buffer = VK_NULL_HANDLE;
		m_Memory = VK_NULL_HANDLE;
	}

	void VKBuffer::Map()
	{
		VK_CHECK_RESULT(vkMapMemory(m_Device->GetSpecification().Device, m_Memory, 0, VK_WHOLE_SIZE, 0, &m_Mapped));
	}

	void VKBuffer::Unmap()
	{
		if (m_Mapped != nullptr)
		{
			vkUnmapMemory(m_Device->GetSpecification().Device, m_Memory);
			m_Mapped = nullptr;
		}
	}

	void VKBuffer::Flush(VkDeviceSize size)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.pNext = nullptr;
		mappedMemoryRange.memory = m_Memory;
		mappedMemoryRange.size = size;
		mappedMemoryRange.offset = 0;
		VK_CHECK_RESULT(vkFlushMappedMemoryRanges(m_Device->GetSpecification().Device, 1, &mappedMemoryRange));
	}
}