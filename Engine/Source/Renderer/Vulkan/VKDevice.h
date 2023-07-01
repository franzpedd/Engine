#pragma once

#include "Core/Defines.h"
#include "VKInstance.h"

#include <vulkan/vulkan.h>

namespace Cosmos
{
	class COSMOS_API VKDevice
	{
	public:

		struct QueueFamilyIndices
		{
			u32 Graphics = 0;
			u32 Compute = 0;
		};

		struct Specification
		{
			VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
			VkDevice Device = VK_NULL_HANDLE;
			VkCommandPool CommandPool = VK_NULL_HANDLE;
			VkQueue Queue = VK_NULL_HANDLE;
			VkPhysicalDeviceProperties Properties = {};
			VkPhysicalDeviceFeatures Features = {};
			VkPhysicalDeviceFeatures EnabledFeatures = {};
			VkPhysicalDeviceMemoryProperties MemoryProperties = {};
			std::vector<VkQueueFamilyProperties> QueueFamilyProperties = {};
			QueueFamilyIndices QueueFamilyIndices = {};
		};

	public:

		// returns a smart-ptr for a vkdevice
		static std::shared_ptr<VKDevice> Create(std::shared_ptr<Platform>& platform, std::shared_ptr<VKInstance>& instance);

		// constructor
		VKDevice(std::shared_ptr<Platform>& platform, std::shared_ptr<VKInstance>& instance);
		
		// destructor
		~VKDevice();

		// returns a reference of the members
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// creates a buffer on the device
		VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);

		// gets the index of a memory type that has all the requested property bits set
		u32 GetMemoryType(u32 bits, VkMemoryPropertyFlags properties, VkBool32* found = nullptr);

		// gets the index of a queue family that supports the requested queue flags
		u32 GetQueueFamilyIndex(VkQueueFlagBits flags);

		// creates a command pool for allocation command
		VkCommandPool CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		// allocate a command buffer from the command pool
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);

		// starts a new command buffer from scratch
		void BeginCommandBuffer(VkCommandBuffer commandBuffer);

		// finish the command buffer recording and send it to a queue
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

	private:

		// selects the physical device
		void SetupDevice();

		// creates the logical device
		void CreateDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, VkQueueFlags queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	private:

		std::shared_ptr<Platform>& m_Platform;
		std::shared_ptr<VKInstance>& m_Instance;
		Specification m_Specification;
	};
}