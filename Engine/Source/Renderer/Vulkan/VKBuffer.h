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
			Index,
			Uniform
		};

	public:

		// returns a smart-ptr to a new vkbuffer
		static std::shared_ptr<VKBuffer> Create(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data = 0);

		// constructor
		VKBuffer(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data = 0);

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
		const void* mData = nullptr; // i.e: vertex, index data, uniform data
	};

	// still used in texture and model class, when testing remove this and rework VKBuffer to support staging only when required
	// creates a buffer on the gpu, used for buffers without staging (as VKBuffer class uses them)
	VkResult BufferCreate(std::shared_ptr<VKDevice>& device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);

	// command buffer will be handled with a class, using functions at the momment
	// starts the recording of a once-used command buffer
	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool);

	// ends the recording of a once-used command buffer
	void EndSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer);

	// creates a command buffer given a command pool (usefull while using swapchain's command buffer)
	VkCommandBuffer CreateCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBufferLevel level, bool begin = false);

	// initializes the command buffer
	void BeginCommandBuffer(VkCommandBuffer cmdBuffer);

	// finishes the recording of a command buffer and send it to the queue
	void FlushCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, bool free = false);
}