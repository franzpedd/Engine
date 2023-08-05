#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	class VKInstance
	{
	public:

		// creates a smart-ptr to a new vkinstance
		static std::shared_ptr<VKInstance> Create(const char* appName, const char* engineName, bool validations = true);

		// constructor
		VKInstance(const char* appName, const char* engineName, bool validations = true);

		// destructor
		~VKInstance();

		// returns a reference to the vulkan instance
		inline VkInstance& Instance() { return mInstance; }

		// returns a reference to the vulkan debug utils messenger
		inline VkDebugUtilsMessengerEXT Debugger() { return mDebugMessenger; }

		// returns if validations are enabled
		inline bool Validations() { return mValidations; }

		// returns listed validations
		inline const std::vector<const char*> ValidationsList() { return mValidationList; }

	public:

		// returns the required extensions by the vkinstance
		std::vector<const char*> GetRequiredExtensions();

	private:

		// internal errors callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserData);

		// hook external function that creates a debug util messenger
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		// hooks external function that deletes a debug util messenger
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	private:

		const char* mAppName = nullptr;
		const char* mEngineName = nullptr;
		bool mValidations = true;
		const std::vector<const char*> mValidationList = { "VK_LAYER_KHRONOS_validation" };
		VkInstance mInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
	};
}