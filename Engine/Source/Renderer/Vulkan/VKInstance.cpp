#include "VKInstance.h"

#include "Platform/Vulkan.h"

#include "Util/Logger.h"

namespace Cosmos
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback, void* pUserData)
	{
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARNING("Validation Layer: %s", pCallback->pMessage);

			return VK_FALSE;
		}

		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_ERROR("Validation Layer: %s", pCallback->pMessage);
			return VK_FALSE;
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) func(instance, debugMessenger, pAllocator);
	}

	std::shared_ptr<VKInstance> VKInstance::Create(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations)
	{
		return std::make_shared<VKInstance>(platform, appName, engineName, validations);
	}

	VKInstance::VKInstance(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations)
		: m_Platform(platform)
	{
		LOG_TRACE("Creating VKInstance Class");

		m_Specification.AppName = appName;
		m_Specification.EngineName = engineName;
		m_Specification.Validations = validations;

		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = m_Specification.AppName;
		applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		applicationInfo.pEngineName = m_Specification.EngineName;
		applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		auto extensions = GetRequiredExtensions();

		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI1{};

		VkInstanceCreateInfo instanceCI{};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		instanceCI.pApplicationInfo = &applicationInfo;

		if (m_Specification.Validations)
		{
			instanceCI.enabledLayerCount = static_cast<uint32_t>(m_Specification.ValidationList.size());
			instanceCI.ppEnabledLayerNames = m_Specification.ValidationList.data();

			debugUtilsMessengerCI1 = {};
			debugUtilsMessengerCI1.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCI1.pNext = nullptr;
			debugUtilsMessengerCI1.flags = 0;
			debugUtilsMessengerCI1.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCI1.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsMessengerCI1.pfnUserCallback = DebugCallback;
			debugUtilsMessengerCI1.pUserData = nullptr;

			instanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCI1;
		}

		else
		{
			instanceCI.pNext = nullptr;
			instanceCI.enabledLayerCount = 0;
			instanceCI.ppEnabledLayerNames = nullptr;
		}

		instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceCI.ppEnabledExtensionNames = extensions.data();

		LOG_ASSERT(vkCreateInstance(&instanceCI, nullptr, &m_Specification.Instance) == VK_SUCCESS, "Failed to create Vulkan Instance");

		if (m_Specification.Validations)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI2{};
			debugUtilsMessengerCI2 = {};
			debugUtilsMessengerCI2.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCI2.pNext = nullptr;
			debugUtilsMessengerCI2.flags = 0;
			debugUtilsMessengerCI2.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCI2.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsMessengerCI2.pfnUserCallback = DebugCallback;
			debugUtilsMessengerCI2.pUserData = nullptr;

			LOG_ASSERT(CreateDebugUtilsMessengerEXT(m_Specification.Instance, &debugUtilsMessengerCI2, nullptr, &m_Specification.DebugMessenger) == VK_SUCCESS, "Failed to create Vulkan Debug Messenger");
		}
	}

	VKInstance::~VKInstance()
	{
		if (m_Specification.Validations)
		{
			DestroyDebugUtilsMessengerEXT(m_Specification.Instance, m_Specification.DebugMessenger, nullptr);
		}

		vkDestroyInstance(m_Specification.Instance, nullptr);
	}

	std::vector<const char*> VKInstance::GetRequiredExtensions()
	{
		std::vector<const char*> extensions = {};
		extensions.push_back(GetRendererAPIRequiredExtensions());
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		
		if (m_Specification.Validations)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		return extensions;
	}

	bool VKInstance::CheckValidationSupport()
	{
		u32 count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);

		std::vector<VkLayerProperties> availableLayers(count);
		vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

		for (const char* layer : m_Specification.ValidationList)
		{
			bool found = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layer, layerProperties.layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				return false;
			}
		}

		return true;
	}
}