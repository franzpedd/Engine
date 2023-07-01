#include "VKSwapchain.h"

#include "Platform/Vulkan.h"

#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<VKSwapchain> VKSwapchain::Create(std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, u32 width, u32 height, bool vsync)
	{
		return std::make_shared<VKSwapchain>(instance, device, width, height, vsync);
	}

	VKSwapchain::VKSwapchain(std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, u32 width, u32 height, bool vsync)
		: m_Instance(instance), m_Device(device)
	{
		LOG_TRACE("Creating VKSwapchain Class");

		m_Specification.Width = width;
		m_Specification.Height = height;
		m_Specification.VSync = vsync;

		InitSurface();
		SetupSwapchain(&m_Specification.Width, &m_Specification.Height, m_Specification.VSync);
	}

	VKSwapchain::~VKSwapchain()
	{
	}
	
	void VKSwapchain::InitSurface()
	{
		VK_CHECK_RESULT(CreateWindowSurface(m_Instance->GetSpecification().Instance, nullptr, &m_Specification.Surface));

		u32 queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device->GetSpecification().PhysicalDevice, &queueCount, nullptr);

		LOG_ASSERT(queueCount >= 1, "We must have at least one queue available");

		std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device->GetSpecification().PhysicalDevice, &queueCount, queueProperties.data());

		// find a queue that supports presenting images for the windowing system
		// prefer one that also supports graphics
		std::vector<VkBool32> supportsPresent(queueCount);

		for(u32 i = 0; i < queueCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetSpecification().PhysicalDevice, i, m_Specification.Surface, &supportsPresent[i]);
		}

		u32 graphicsQueueNodeIndex = UINT32_MAX;
		u32 presentQueueNodeIndex = UINT32_MAX;

		for (u32 i = 0; i < queueCount; i++)
		{
			if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;

					break;
				}
			}
		}

		// there's no queue that supports both present and graphics, try to find a separate one
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			for (u32 i = 0; i < queueCount; i++)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		LOG_ASSERT(graphicsQueueNodeIndex != UINT32_MAX, "Failed to find a Graphics Queue");
		LOG_ASSERT(presentQueueNodeIndex != UINT32_MAX, "Failed to find a Presentation Queue");

		m_Specification.QueueNodeIndex = graphicsQueueNodeIndex;

		// gets a list of supported formats
		u32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetSpecification().PhysicalDevice, m_Specification.Surface, &formatCount, nullptr);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetSpecification().PhysicalDevice, m_Specification.Surface, &formatCount, surfaceFormats.data());

		// if surface format only has VK_FORMAT_UNDEFINED there is no prefered format, so we assume VK_FORMAT_B8G8R8A8_SRGB
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_Specification.ColorFormat = VK_FORMAT_B8G8R8A8_SRGB;
			m_Specification.ColorSpace = surfaceFormats[0].colorSpace;
		}

		else
		{
			// iterate over the list of available surface format and check for the presence of VK_FORMAT_B8G8R8A8_SRGB
			bool found = false;

			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
				{
					m_Specification.ColorFormat = surfaceFormat.format;
					m_Specification.ColorSpace = surfaceFormat.colorSpace;
					found = true;

					break;
				}
			}

			// VK_FORMAT_B8G8R8A8_SRGB is not available, select the first one
			if (!found)
			{
				m_Specification.ColorFormat = surfaceFormats[0].format;
				m_Specification.ColorSpace = surfaceFormats[0].colorSpace;
			}
		}

		// find suitable depth format
		std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
		VkBool32 foundDepthFormat = false;

		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(m_Device->GetSpecification().PhysicalDevice, format, &formatProps);

			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_Specification.DepthFormat = format;
				foundDepthFormat = true;

				break;
			}
		}

		LOG_ASSERT(foundDepthFormat == VK_TRUE, "Failed to find depth format");

		return;
	}

	void VKSwapchain::SetupSwapchain(u32* width, u32* height, bool vSync)
	{
		VkSwapchainKHR oldSwapchain = m_Specification.Swapchain;

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetSpecification().PhysicalDevice, m_Specification.Surface, &surfaceCapabilities);

		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetSpecification().PhysicalDevice, m_Specification.Surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetSpecification().PhysicalDevice, m_Specification.Surface, &presentModeCount, presentModes.data());

		LOG_ASSERT(presentModeCount > 0, "There's no present mode available for the swapchain");

		// if value is 0xFFFFFFFF the size of the surface will be set by the swapchain
		if (surfaceCapabilities.currentExtent.width = (u32)-1)
		{
			// surface size is undefined, setting it to the size of the images required
			m_Specification.Extent.width = *width;
			m_Specification.Extent.height = *height;
		}

		else
		{
			// swapchain size is defined, images size must match
			m_Specification.Extent = surfaceCapabilities.currentExtent;
			*width = surfaceCapabilities.currentExtent.width;
			*height = surfaceCapabilities.currentExtent.height;
		}

		// select present mode for the swapchain
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR; // always present

		// vsync is not requested, try to find mailbox as it's the best mode available
		if (!vSync)
		{
			for (u32 i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// determines the number of swapchain images
		u32 desiredNumbersOfSwapchainImages = surfaceCapabilities.minImageCount + 1;

		if (surfaceCapabilities.maxImageCount > 0 && desiredNumbersOfSwapchainImages > surfaceCapabilities.maxImageCount)
		{
			desiredNumbersOfSwapchainImages = surfaceCapabilities.maxImageCount;
		}

		// find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;

		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // prefer non-rotated transform
		}

		else
		{
			preTransform = surfaceCapabilities.currentTransform;
		}

		// find a supported composite alpha format (not all devices supports alpha opaque)
		// select the first available
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags =
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			}
		}

		// create swapchain
		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = nullptr;
		swapchainCI.surface = m_Specification.Surface;
		swapchainCI.minImageCount = desiredNumbersOfSwapchainImages;
		swapchainCI.imageFormat = m_Specification.ColorFormat;
		swapchainCI.imageColorSpace = m_Specification.ColorSpace;
		swapchainCI.imageExtent = { m_Specification.Extent.width, m_Specification.Extent.height };
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = nullptr;
		swapchainCI.presentMode = swapchainPresentMode;
		swapchainCI.oldSwapchain = oldSwapchain;
		swapchainCI.clipped = VK_TRUE; // discart rendering outside surface area
		swapchainCI.compositeAlpha = compositeAlpha;

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_Device->GetSpecification().PhysicalDevice, m_Specification.ColorFormat, &formatProperties);

		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR || formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)
		{
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Device->GetSpecification().Device, &swapchainCI, nullptr, &m_Specification.Swapchain));

		// if existing swapchain is re-created, destroy old one
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_Specification.ImageCount; i++)
			{
				vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.Buffers[i].View, nullptr);
			}
			vkDestroySwapchainKHR(m_Device->GetSpecification().Device, m_Specification.Swapchain, nullptr);
		}

		// get the swap chain images
		vkGetSwapchainImagesKHR(m_Device->GetSpecification().Device, m_Specification.Swapchain, &m_Specification.ImageCount, nullptr);
		m_Specification.Images.resize(m_Specification.ImageCount);
		vkGetSwapchainImagesKHR(m_Device->GetSpecification().Device, m_Specification.Swapchain, &m_Specification.ImageCount, m_Specification.Images.data());

		// get the swap chain buffers containing the image and imageview
		m_Specification.Buffers.resize(m_Specification.ImageCount);
		for (uint32_t i = 0; i < m_Specification.ImageCount; i++)
		{
			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCI.pNext = nullptr;
			imageViewCI.format = m_Specification.ColorFormat;
			imageViewCI.components =
			{
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCI.subresourceRange.baseMipLevel = 0;
			imageViewCI.subresourceRange.levelCount = 1;
			imageViewCI.subresourceRange.baseArrayLayer = 0;
			imageViewCI.subresourceRange.layerCount = 1;
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.flags = 0;

			m_Specification.Buffers[i].Image = m_Specification.Images[i];
			imageViewCI.image = m_Specification.Buffers[i].Image;

			VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.Buffers[i].View));
		}
	}

	VkResult VKSwapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore, u32* imageIndex)
	{
		if (m_Specification.Swapchain != VK_NULL_HANDLE)
		{
			return VK_ERROR_OUT_OF_DATE_KHR;
		}

		return vkAcquireNextImageKHR(m_Device->GetSpecification().Device, m_Specification.Swapchain, UINT64_MAX, presentCompleteSemaphore, nullptr, imageIndex);
	}

	VkResult VKSwapchain::QueuePresent(VkQueue queue, u32 imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Specification.Swapchain;
		presentInfo.pImageIndices = &imageIndex;
		
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}

		return vkQueuePresentKHR(queue, &presentInfo);
	}

	void VKSwapchain::Cleanup()
	{
		if (m_Specification.Swapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_Specification.ImageCount; i++)
			{
				vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.Buffers[i].View, nullptr);
			}
		}

		if (m_Specification.Surface != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_Device->GetSpecification().Device, m_Specification.Swapchain, nullptr);
			vkDestroySurfaceKHR(m_Instance->GetSpecification().Instance, m_Specification.Surface, nullptr);
		}
	}
}