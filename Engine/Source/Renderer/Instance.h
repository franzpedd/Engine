#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	class Instance
	{
	public:

		// constructor
		Instance() = default;

		// destructor
		virtual ~Instance() = default;

	public:

		// returns a reference to the vulkan instance
		virtual VkInstance GetInstance() = 0;

		// returns a reference to the vulkan debug utils messenger
		virtual VkDebugUtilsMessengerEXT GetDebugger() = 0;

		// returns if validations are enabled
		virtual bool GetValidations() = 0;

		// returns listed validations
		virtual const std::vector<const char*> GetValidationsList() = 0;

		// returns the required extensions by the vkinstance
		virtual std::vector<const char*> GetRequiredExtensions() = 0;
	};
}