#include "VKSwapchain.h"

#include "VKDevice.h"
#include "VKInstance.h"
#include "VKUtility.h"
#include "Platform/Window.h"
#include "Util/Logger.h"

#include <algorithm>
#include <array>
#include <vector>

namespace Cosmos
{
	std::shared_ptr<VKSwapchain> VKSwapchain::Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device)
	{
		return std::make_shared<VKSwapchain>(window, instance, device);
	}

	VKSwapchain::VKSwapchain(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device)
		: mWindow(window), mDevice(device), mInstance(instance)
	{
		Logger() << "Creating VKSwapchain";

		CreateSwapchain();
		CreateImageViews();
	}

	VKSwapchain::~VKSwapchain()
	{
		Cleanup();
	}

	void VKSwapchain::CreateSwapchain()
	{
		Details details = QueryDetails();

		// query all important details of the swapchain
		{
			mSurfaceFormat = ChooseSurfaceFormat(details.formats);
			mPresentMode = ChoosePresentMode(details.presentModes);
			mExtent = ChooseExtent(details.capabilities);

			mImageCount = details.capabilities.minImageCount + 1;
			if (details.capabilities.maxImageCount > 0 && mImageCount > details.capabilities.maxImageCount)
			{
				mImageCount = details.capabilities.maxImageCount;
			}
		}

		// create the swapchain
		{
			QueueFamilyIndices indices = mDevice->FindQueueFamilies(mDevice->PhysicalDevice(), mDevice->Surface());
			uint32_t queueFamilyIndices[] = { indices.graphics.value(), indices.present.value() };

			VkSwapchainCreateInfoKHR swapchainCI = {};
			swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCI.pNext = nullptr;
			swapchainCI.flags = 0;
			swapchainCI.surface = mDevice->Surface();
			swapchainCI.minImageCount = mImageCount;
			swapchainCI.imageFormat = mSurfaceFormat.format;
			swapchainCI.imageColorSpace = mSurfaceFormat.colorSpace;
			swapchainCI.imageExtent = mExtent;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			if (indices.graphics != indices.present)
			{
				swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCI.queueFamilyIndexCount = 2;
				swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
			}

			else
			{
				swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}

			swapchainCI.preTransform = details.capabilities.currentTransform;
			swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapchainCI.presentMode = mPresentMode;
			swapchainCI.clipped = VK_TRUE;

			LOG_ASSERT(vkCreateSwapchainKHR(mDevice->Device(), &swapchainCI, nullptr, &mSwapchain) == VK_SUCCESS, "Swapchain: Failed to create the Swapchain");

			// get the images in the swapchain
			vkGetSwapchainImagesKHR(mDevice->Device(), mSwapchain, &mImageCount, nullptr);
			mImages.resize(mImageCount);
			vkGetSwapchainImagesKHR(mDevice->Device(), mSwapchain, &mImageCount, mImages.data());
		}
	}

	void VKSwapchain::CreateImageViews()
	{
		mImageViews.resize(mImages.size());

		for (size_t i = 0; i < mImages.size(); i++)
		{
			mImageViews[i] = CreateImageView(mDevice, mImages[i], mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VKSwapchain::CreateFramebuffers(VkRenderPass& renderPass, VkSampleCountFlagBits msaa)
	{
		// create frame color resources
		{
			VkFormat format = mSurfaceFormat.format;

			CreateImage
			(
				mDevice,
				mExtent.width,
				mExtent.height,
				1,
				msaa,
				format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mColorImage,
				mColorMemory
			);

			mColorView = CreateImageView(mDevice, mColorImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		// create frame depth resources
		{
			VkFormat format = FindDepthFormat(mDevice);

			CreateImage
			(
				mDevice,
				mExtent.width,
				mExtent.height,
				1,
				msaa,
				format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = CreateImageView(mDevice, mDepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		}

		// create frame buffers
		{
			mFramebuffers.resize(mImageViews.size());

			for (size_t i = 0; i < mImageViews.size(); i++)
			{
				std::array<VkImageView, 3> attachments = { mColorView, mDepthView, mImageViews[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderPass;
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = mExtent.width;
				framebufferCI.height = mExtent.height;
				framebufferCI.layers = 1;
				LOG_ASSERT(vkCreateFramebuffer(mDevice->Device(), &framebufferCI, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}
	}

	void VKSwapchain::Cleanup()
	{
		vkDestroyImageView(mDevice->Device(), mDepthView, nullptr);
		vkDestroyImage(mDevice->Device(), mDepthImage, nullptr);
		vkFreeMemory(mDevice->Device(), mDepthMemory, nullptr);

		vkDestroyImageView(mDevice->Device(), mColorView, nullptr);
		vkDestroyImage(mDevice->Device(), mColorImage, nullptr);
		vkFreeMemory(mDevice->Device(), mColorMemory, nullptr);

		for (uint32_t i = 0; i < mFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(mDevice->Device(), mFramebuffers[i], nullptr);
		}

		for (auto imageView : mImageViews)
		{
			vkDestroyImageView(mDevice->Device(), imageView, nullptr);
		}
		
		vkDestroySwapchainKHR(mDevice->Device(), mSwapchain, nullptr);
	}

	void VKSwapchain::Recreate(VkRenderPass& renderPass, VkSampleCountFlagBits msaa)
	{
		int width = 0;
		int height = 0;
		mWindow->GetFramebufferSize(&width, &height);

		while (width == 0 || height == 0)
		{
			mWindow->GetFramebufferSize(&width, &height);
			mWindow->WaitEvents();
		}

		vkDeviceWaitIdle(mDevice->Device());

		Cleanup();

		CreateSwapchain();
		CreateImageViews();
		CreateFramebuffers(renderPass, msaa);
	}

	VKSwapchain::Details VKSwapchain::QueryDetails()
	{
		Details details = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->PhysicalDevice(), mDevice->Surface(), &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->PhysicalDevice(), mDevice->Surface(), &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->PhysicalDevice(), mDevice->Surface(), &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->PhysicalDevice(), mDevice->Surface(), &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->PhysicalDevice(), mDevice->Surface(), &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VKSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VKSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for(const auto & availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VKSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		else
		{
			int width, height;
			mWindow->GetFramebufferSize(&width, &height);

			VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}