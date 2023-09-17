#pragma once

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declarations
	class VKDevice;

	class VKBuffer
	{
	public:

		enum Type
		{
			Vertex = 0,
			Index
		};

	public:

		// returns a smart-ptr to a new vkbuffer
		static std::shared_ptr<VKBuffer> Create(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data);

		// constructor
		VKBuffer(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data);

		// destructor
		~VKBuffer() = default;

		// returns a reference to the buffer
		inline VkBuffer& Buffer() { return mBuffer; }

		// returns a copy of the buffer size
		inline VkDeviceSize GetSize() { return mSize; }

	public:

		// free used resources
		void Destroy();

		// creates a buffer on gpu using staging buffer
		void CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);

		// copies the buffer to another
		void CopyBuffer(VkBuffer src, VkBuffer dst);

	private:

		std::shared_ptr<VKDevice>& mDevice;
		VkCommandPool& mCmdPool;
		Type mType;
		VkDeviceSize mSize;
		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mBufferMemory = VK_NULL_HANDLE;
		const void* mData = nullptr; // i.e: vertex, index data
	};
}