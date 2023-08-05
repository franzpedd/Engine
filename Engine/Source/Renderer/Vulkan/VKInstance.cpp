#include "VKInstance.h"

#include "Platform/Window.h"
#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<VKInstance> VKInstance::Create(const char* appName, const char* engineName, bool validations)
	{
		return std::make_shared<VKInstance>(appName, engineName, validations);
	}

	VKInstance::VKInstance(const char* appName, const char* engineName, bool validations)
	{
		Logger() << "Creating VKInstance";

		mAppName = appName;
		mEngineName = engineName;
		mValidations = validations;

		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = appName;
		applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		applicationInfo.pEngineName = engineName;
		applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		auto extensions = GetRequiredExtensions();

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCI = {};
		VkInstanceCreateInfo instanceCI = {};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		instanceCI.pApplicationInfo = &applicationInfo;

		if (validations)
		{
			instanceCI.enabledLayerCount = (uint32_t)mValidationList.size();
			instanceCI.ppEnabledLayerNames = mValidationList.data();

			debugUtilsCI = {};
			debugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsCI.pNext = nullptr;
			debugUtilsCI.flags = 0;
			debugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCI.pfnUserCallback = DebugCallback;
			debugUtilsCI.pUserData = nullptr;

			instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsCI;
		}

		else
		{
			instanceCI.pNext = nullptr;
			instanceCI.enabledLayerCount = 0;
			instanceCI.ppEnabledLayerNames = nullptr;
		}

		instanceCI.enabledExtensionCount = (uint32_t)extensions.size();
		instanceCI.ppEnabledExtensionNames = extensions.data();
		LOG_ASSERT(vkCreateInstance(&instanceCI, nullptr, &mInstance) == VK_SUCCESS, "Failed to create Vulkan Instance");

		if (validations)
		{
			debugUtilsCI = {};
			debugUtilsCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsCI.pNext = nullptr;
			debugUtilsCI.flags = 0;
			debugUtilsCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsCI.pfnUserCallback = DebugCallback;
			debugUtilsCI.pUserData = nullptr;

			LOG_ASSERT(CreateDebugUtilsMessengerEXT(mInstance, &debugUtilsCI, nullptr, &mDebugMessenger) == VK_SUCCESS, "Failed to create Vulkan Debug Messenger");
		}
	}

	VKInstance::~VKInstance()
	{
		if (mValidations)
		{
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroyInstance(mInstance, nullptr);
	}

	std::vector<const char*> VKInstance::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = Window::GetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		if (mValidations)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		}
		
		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VKInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserData)
	{
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_TO_TERMINAL(Logger::Severity::Error, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LOG_TO_TERMINAL(Logger::Severity::Info, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LOG_TO_TERMINAL(Logger::Severity::Trace, "Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		return VK_FALSE;
	}

	VkResult VKInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void VKInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}
}