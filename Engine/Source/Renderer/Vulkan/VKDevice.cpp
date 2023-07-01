#include "VKDevice.h"

namespace Cosmos
{
	std::shared_ptr<VKDevice> VKDevice::Create(std::shared_ptr<Platform>& platform, std::shared_ptr<VKInstance>& instance)
	{
		return std::make_shared<VKDevice>(platform, instance);
	}

	VKDevice::VKDevice(std::shared_ptr<Platform>& platform, std::shared_ptr<VKInstance>& instance)
		: m_Platform(platform), m_Instance(instance)
	{
		LOG_TRACE("Creating VKDevice Class");

		SetupDevice();

		std::vector<const char*> enabledExtensions{};
		VkPhysicalDeviceFeatures enabledFeatures{};

		if (m_Specification.Features.samplerAnisotropy)
		{
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		}

		CreateDevice(enabledFeatures, enabledExtensions);
	}

	VKDevice::~VKDevice()
	{
		if (m_Specification.CommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_Specification.Device, m_Specification.CommandPool, nullptr);
		}

		if (m_Specification.Device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(m_Specification.Device, nullptr);
		}
	}

	VkResult VKDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// creates the buffer handler
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.pNext = nullptr;
		bufferCI.usage = usageFlags;
		bufferCI.size = size;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(m_Specification.Device, &bufferCI, nullptr, buffer));

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_Specification.Device, *buffer, &memoryRequirements);

		// creates the buffer memory
		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = GetMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);
		VK_CHECK_RESULT(vkAllocateMemory(m_Specification.Device, &memoryAllocateInfo, nullptr, memory));
		
		if (data != nullptr)
		{
			void* mapped;
			VK_CHECK_RESULT(vkMapMemory(m_Specification.Device, *memory, 0, size, 0, &mapped));

			std::memcpy(mapped, data, size);

			// host coherent wasnt requested, manually flush to make writes visible
			if ((propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedMemoryRange = {};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.pNext = nullptr;
				mappedMemoryRange.memory = *memory;
				mappedMemoryRange.offset = 0;
				mappedMemoryRange.size = size;
				vkFlushMappedMemoryRanges(m_Specification.Device, 1, &mappedMemoryRange);
			}

			vkUnmapMemory(m_Specification.Device, *memory);
		}
		
		// attach memory to the buffer object
		VK_CHECK_RESULT(vkBindBufferMemory(m_Specification.Device, *buffer, *memory, 0));

		return VK_SUCCESS;
	}

	u32 VKDevice::GetMemoryType(u32 bits, VkMemoryPropertyFlags properties, VkBool32* found)
	{
		for (u32 i = 0; i < m_Specification.MemoryProperties.memoryTypeCount; i++)
		{
			if ((bits & 1) == 1)
			{
				if ((m_Specification.MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (found)
					{
						*found = true;
					}

					return i;
				}
			}
			bits >>= 1;
		}

		if (found)
		{
			*found = false;
			return 0;
		}
		
		LOG_ASSERT(false, "Vulkan Could not find a matching memory type");
		return 0;
	}

	u32 VKDevice::GetQueueFamilyIndex(VkQueueFlagBits flags)
	{
		// try to find a queue family index that supports compute but not graphics
		if (flags & VK_QUEUE_COMPUTE_BIT)
		{
			for (u32 i = 0; i < (u32)m_Specification.QueueFamilyProperties.size(); i++)
			{
				if ((m_Specification.QueueFamilyProperties[i].queueFlags & flags) && ((m_Specification.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		// for other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (u32 i = 0; i < (u32)m_Specification.QueueFamilyProperties.size(); i++)
		{
			if (m_Specification.QueueFamilyProperties[i].queueFlags & flags)
			{
				return i;
			}
		}

		LOG_ASSERT(false, "Could not find a matching Queue Family Index");
		return 0;
	}

	VkCommandPool VKDevice::CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo commandPoolCI = {};
		commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCI.pNext = nullptr;
		commandPoolCI.queueFamilyIndex = queueFamilyIndex;
		commandPoolCI.flags = flags;

		VkCommandPool commandPool;
		VK_CHECK_RESULT(vkCreateCommandPool(m_Specification.Device, &commandPoolCI, nullptr, &commandPool));

		return commandPool;
	}

	VkCommandBuffer VKDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.level = level;
		commandBufferAllocateInfo.commandPool = m_Specification.CommandPool;
		commandBufferAllocateInfo.commandBufferCount = 1;
		
		VkCommandBuffer commandBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Specification.Device, &commandBufferAllocateInfo, &commandBuffer));

		if (begin)
		{
			VkCommandBufferBeginInfo commandBufferBeginInfo = {};
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.pNext = nullptr;
			
			VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		}

		return commandBuffer;
	}

	void VKDevice::BeginCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
	}

	void VKDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkFence fence;
		VkFenceCreateInfo fenceCI = {};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		
		// create fence to ensure command buffer has finished
		VK_CHECK_RESULT(vkCreateFence(m_Specification.Device, &fenceCI, nullptr, &fence));

		// submit to the queue
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

		// wait fence to signal the command buffer has finished
		VK_CHECK_RESULT(vkWaitForFences(m_Specification.Device, 1, &fence, VK_TRUE, 100000000000));

		vkDestroyFence(m_Specification.Device, fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(m_Specification.Device, m_Specification.CommandPool, 1, &commandBuffer);
		}
	}

	void VKDevice::SetupDevice()
	{
		u32 physicalDevicesCount = 0;
		vkEnumeratePhysicalDevices(m_Instance->GetSpecification().Instance, &physicalDevicesCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
		vkEnumeratePhysicalDevices(m_Instance->GetSpecification().Instance, &physicalDevicesCount, physicalDevices.data());

		LOG_ASSERT(physicalDevicesCount > 0, "Vulkan could not find any GPU");

		u32 selectedPhysicalDevice = 0;
		m_Specification.PhysicalDevice = physicalDevices[selectedPhysicalDevice]; // selecting first one available right now

		// store properties features, limits and properties of the physical device for later use
		vkGetPhysicalDeviceProperties(m_Specification.PhysicalDevice, &m_Specification.Properties);
		vkGetPhysicalDeviceFeatures(m_Specification.PhysicalDevice, &m_Specification.Features);
		vkGetPhysicalDeviceMemoryProperties(m_Specification.PhysicalDevice, &m_Specification.MemoryProperties);

		// queue family properties, used for setting up requested queues upon device creation
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Specification.PhysicalDevice, &queueFamilyCount, nullptr);

		LOG_ASSERT(queueFamilyCount > 0, "Vulkan could not find any Queue");

		m_Specification.QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Specification.PhysicalDevice, &queueFamilyCount, m_Specification.QueueFamilyProperties.data());
	}

	void VKDevice::CreateDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, VkQueueFlags queues)
	{
		// desired queues need to be requested upon logical device creation

		const float defaultQueuePriority(0.0f);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};

		// graphics queue
		if (queues & VK_QUEUE_GRAPHICS_BIT)
		{
			m_Specification.QueueFamilyIndices.Graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

			VkDeviceQueueCreateInfo deviceQueueCI = {};
			deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCI.pNext = nullptr;
			deviceQueueCI.queueFamilyIndex = m_Specification.QueueFamilyIndices.Graphics;
			deviceQueueCI.queueCount = 1;
			deviceQueueCI.pQueuePriorities = &defaultQueuePriority;
			
			queueCreateInfos.push_back(deviceQueueCI);
		}

		else
		{
			m_Specification.QueueFamilyIndices.Graphics = 0;
		}

		// compute queue
		if (queues & VK_QUEUE_COMPUTE_BIT)
		{
			m_Specification.QueueFamilyIndices.Compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

			// compute queue is different from graphics, need a different deviceQueueCI
			if (m_Specification.QueueFamilyIndices.Compute != m_Specification.QueueFamilyIndices.Graphics)
			{
				VkDeviceQueueCreateInfo deviceQueueCI = {};
				deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				deviceQueueCI.pNext = nullptr;
				deviceQueueCI.queueFamilyIndex = m_Specification.QueueFamilyIndices.Compute;
				deviceQueueCI.queueCount = 1;
				deviceQueueCI.pQueuePriorities = &defaultQueuePriority;

				queueCreateInfos.push_back(deviceQueueCI);
			}
		}

		else
		{
			// use the same queue if compute bit was not requested
			m_Specification.QueueFamilyIndices.Compute = m_Specification.QueueFamilyIndices.Graphics;
		}

		std::vector<const char*> deviceExtensions(enabledExtensions);
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.pNext = nullptr;
		deviceCI.queueCreateInfoCount = (u32)queueCreateInfos.size();
		deviceCI.pQueueCreateInfos = queueCreateInfos.data();
		deviceCI.pEnabledFeatures = &enabledFeatures;

		if (deviceExtensions.size() > 0)
		{
			deviceCI.enabledExtensionCount = (u32)deviceExtensions.size();
			deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
		}

		VK_CHECK_RESULT(vkCreateDevice(m_Specification.PhysicalDevice, &deviceCI, nullptr, &m_Specification.Device));

		// set graphics queue
		vkGetDeviceQueue(m_Specification.Device, m_Specification.QueueFamilyIndices.Graphics, 0, &m_Specification.Queue);

		m_Specification.CommandPool = CreateCommandPool(m_Specification.QueueFamilyIndices.Graphics);
		m_Specification.EnabledFeatures = enabledFeatures;
	}
}