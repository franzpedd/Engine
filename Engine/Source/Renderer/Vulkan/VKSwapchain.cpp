#include "epch.h"
#include "VKSwapchain.h"

#include "VKDevice.h"
#include "VKInstance.h"
#include "VKImage.h"
#include "Platform/Window.h"
#include "Renderer/Commander.h"

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

		CreateRenderPass();
		CreateFramebuffers();
	}

	VKSwapchain::~VKSwapchain()
	{
		vkDestroyImageView(mDevice->Device(), mDepthView, nullptr);
		vkDestroyImage(mDevice->Device(), mDepthImage, nullptr);
		vkFreeMemory(mDevice->Device(), mDepthMemory, nullptr);

		vkDestroyImageView(mDevice->Device(), mColorView, nullptr);
		vkDestroyImage(mDevice->Device(), mColorImage, nullptr);
		vkFreeMemory(mDevice->Device(), mColorMemory, nullptr);

		for (auto imageView : mImageViews)
		{
			vkDestroyImageView(mDevice->Device(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(mDevice->Device(), mSwapchain, nullptr);
	}

	void VKSwapchain::CreateRenderPass()
	{
		// attachments descriptions
		std::array<VkAttachmentDescription, 3> attachments = {};

		// color
		attachments[0].format = mSurfaceFormat.format;
		attachments[0].samples = mDevice->MSAA();
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// depth
		attachments[1].format = FindDepthFormat(mDevice);
		attachments[1].samples = mDevice->MSAA();
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// resolve
		attachments[2].format = mSurfaceFormat.format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// finalLayout should not be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as ui is a post render pass that will present

		// attachments references
		std::array<VkAttachmentReference, 3> references = {};

		references[0].attachment = 0;
		references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		references[1].attachment = 1;
		references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		references[2].attachment = 2;
		references[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &references[0];
		subpass.pDepthStencilAttachment = &references[1];
		subpass.pResolveAttachments = &references[2];

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCI = {};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = (uint32_t)attachments.size();
		renderPassCI.pAttachments = attachments.data();
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpass;
		renderPassCI.dependencyCount = 1;
		renderPassCI.pDependencies = &dependency;
		VK_ASSERT(vkCreateRenderPass(mDevice->Device(), &renderPassCI, nullptr, &mDevice->MainCommandEntry()->renderPass), "Failed to create render pass");
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
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // for copying the images, allowing viewports

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

			VK_ASSERT(vkCreateSwapchainKHR(mDevice->Device(), &swapchainCI, nullptr, &mSwapchain), "Swapchain: Failed to create the Swapchain");

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
			mImageViews[i] = CreateImageView(mDevice, mImages[i], mSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void VKSwapchain::CreateFramebuffers()
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
				mDevice->MSAA(),
				format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mColorImage,
				mColorMemory
			);

			mColorView = CreateImageView(mDevice, mColorImage, format, VK_IMAGE_ASPECT_COLOR_BIT);
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
				mDevice->MSAA(),
				format,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = CreateImageView(mDevice, mDepthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		// create frame buffers
		{
			mDevice->MainCommandEntry()->framebuffers.resize(mImageViews.size());

			for (size_t i = 0; i < mImageViews.size(); i++)
			{
				std::array<VkImageView, 3> attachments = { mColorView, mDepthView, mImageViews[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = mDevice->MainCommandEntry()->renderPass;
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = mExtent.width;
				framebufferCI.height = mExtent.height;
				framebufferCI.layers = 1;
				VK_ASSERT(vkCreateFramebuffer(mDevice->Device(), &framebufferCI, nullptr, &mDevice->MainCommandEntry()->framebuffers[i]), "Failed to create framebuffer");
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

		for (uint32_t i = 0; i < mDevice->MainCommandEntry()->framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(mDevice->Device(), mDevice->MainCommandEntry()->framebuffers[i], nullptr);
		}

		for (auto imageView : mImageViews)
		{
			vkDestroyImageView(mDevice->Device(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(mDevice->Device(), mSwapchain, nullptr);
	}

	void VKSwapchain::Recreate()
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
		CreateFramebuffers();
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
		{ // VK_FORMAT_B8G8R8A8_UNORM VK_FORMAT_B8G8R8A8_SRGB
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VKSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // triple-buffer
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_IMMEDIATE_KHR; // render as is
		//return VK_PRESENT_MODE_FIFO_KHR; // vsync
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