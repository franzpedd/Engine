#pragma once

#include "Core/Defines.h"
#include "Platform/Platform.h"
#include "Util/Logger.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Cosmos
{
	class COSMOS_API VKInstance
	{
	public:

		struct Specification
		{
			const char* AppName = nullptr;
			const char* EngineName = nullptr;
			bool Validations = true;
			const std::vector<const char*> ValidationList = { "VK_LAYER_KHRONOS_validation" };
			VkInstance Instance = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
		};

	public:

		// creates a smart-ptr to a new vkinstance
		static std::shared_ptr<VKInstance> Create(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations = true);

		// constructor
		VKInstance(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations = true);

		// destructor
		~VKInstance();

		// returns a reference to the members
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// returns the required extensions by the vkinstance
		std::vector<const char*> GetRequiredExtensions();

		// returns if validation layers are supported
		bool CheckValidationSupport();

	private:

		std::shared_ptr<Platform>& m_Platform;
		Specification m_Specification;

	};
}

#define VK_CHECK_RESULT(f)					\
{											\
	VkResult res = (f);						\
	if (res != VK_SUCCESS)					\
	{										\
		LOG_ERROR("Vulkan Error: %d", res);	\
	}										\
}