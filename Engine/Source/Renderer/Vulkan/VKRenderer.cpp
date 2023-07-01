#include "VKRenderer.h"

#include "Util/Logger.h"

#include <array>

namespace Cosmos
{
	std::shared_ptr<VKRenderer> VKRenderer::Create(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations)
	{
		return std::make_shared<VKRenderer>(platform, appName, engineName, validations);
	}

	VKRenderer::VKRenderer(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations)
		: m_Platform(platform)
	{
		LOG_TRACE("Initializing Vulkan Renderer");

		m_Specification.Validations = validations;
		m_Specification.AppName = appName;
		m_Specification.EngineName = engineName;

		m_Instance = VKInstance::Create(m_Platform, appName, engineName, validations);
		m_Device = VKDevice::Create(m_Platform, m_Instance);
		m_Swapchain = VKSwapchain::Create(m_Instance, m_Device, platform->GetSize().first, platform->GetSize().second);
	}

	VKRenderer::~VKRenderer()
	{
		vkFreeCommandBuffers(m_Device->GetSpecification().Device, m_Specification.CommandPool, (u32)m_Specification.CommandBuffers.size(), m_Specification.CommandBuffers.data());

		for (size_t i = 0; i < m_Specification.Syncronization.RenderAhead; i++)
		{
			vkDestroyFence(m_Device->GetSpecification().Device, m_Specification.Syncronization.WaitFences[i], nullptr);
			vkDestroySemaphore(m_Device->GetSpecification().Device, m_Specification.Syncronization.RenderCompleteSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device->GetSpecification().Device, m_Specification.Syncronization.PresentationCompleteSemaphore[i], nullptr);
		}

		m_Swapchain->Cleanup();

		vkDestroyDescriptorPool(m_Device->GetSpecification().Device, m_Specification.DescriptorPool, nullptr);
		vkDestroyRenderPass(m_Device->GetSpecification().Device, m_Specification.RenderPass, nullptr);

		for (u32 i = 0; i < m_Specification.FrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_Device->GetSpecification().Device, m_Specification.FrameBuffers[i], nullptr);
		}

		vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.DepthStencil.view, nullptr);
		vkDestroyImage(m_Device->GetSpecification().Device, m_Specification.DepthStencil.image, nullptr);
		vkFreeMemory(m_Device->GetSpecification().Device, m_Specification.DepthStencil.memory, nullptr);

		vkDestroyPipelineCache(m_Device->GetSpecification().Device, m_Specification.PipelineCache, nullptr);
		vkDestroyCommandPool(m_Device->GetSpecification().Device, m_Specification.CommandPool, nullptr);

		if (m_Settings.Multisampling)
		{
			vkDestroyImage(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Color.image, nullptr);
			vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Color.view, nullptr);
			vkFreeMemory(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Color.memory, nullptr);
			vkDestroyImage(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Depth.image, nullptr);
			vkDestroyImageView(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Depth.view, nullptr);
			vkFreeMemory(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Depth.memory, nullptr);
		}
	}

	void VKRenderer::Prepare()
	{
		LOG_TRACE("Preparing VKRenderer");

		CreateCommandPool();
		CreateRenderPass();
		CreateSyncSystem();
		CreateCommandBuffers();

		m_DescriptorSets.resize(m_Swapchain->GetSpecification().ImageCount);
		m_UniformBuffers.resize(m_Swapchain->GetSpecification().ImageCount);
	}

	void VKRenderer::Render()
	{

	}

	void VKRenderer::CreateCommandPool()
	{
		VkCommandPoolCreateInfo commandPoolCI = {};
		commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCI.queueFamilyIndex = m_Swapchain->GetSpecification().QueueNodeIndex;
		commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCI.pNext = nullptr;

		VK_CHECK_RESULT(vkCreateCommandPool(m_Device->GetSpecification().Device, &commandPoolCI, nullptr, &m_Specification.CommandPool));
	}

	void VKRenderer::CreateRenderPass()
	{
		if (m_Settings.Multisampling)
		{
			std::array<VkAttachmentDescription, 4> attachments = {};

			// multsampled attachment we want render to
			attachments[0].format = m_Swapchain->GetSpecification().ColorFormat;
			attachments[0].samples = m_Settings.Samples;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// framebuffer attachment the multisampled image will be resolved to be presented in the swapchain
			attachments[1].format = m_Swapchain->GetSpecification().ColorFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// multisampled depth attachment we render to
			attachments[2].format = m_Swapchain->GetSpecification().DepthFormat;
			attachments[2].samples = m_Settings.Samples;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// depth resolve attachment
			attachments[3].format = m_Swapchain->GetSpecification().DepthFormat;
			attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// attachments reference
			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 2;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference resolveReference = {};
			resolveReference.attachment = 1;
			resolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			
			// subpass
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorReference;
			subpass.pResolveAttachments = &resolveReference;
			subpass.pDepthStencilAttachment = &depthReference;
			
			// subpass dependencies
			std::array<VkSubpassDependency, 2> dependencies = {};

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.pNext = nullptr;
			renderPassCI.attachmentCount = (u32)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = (u32)dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			VK_CHECK_RESULT(vkCreateRenderPass(m_Device->GetSpecification().Device, &renderPassCI, nullptr, &m_Specification.RenderPass));
		}

		else
		{
			std::array<VkAttachmentDescription, 2> attachments = {};

			// color attachment
			attachments[0].format = m_Swapchain->GetSpecification().ColorFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// depth attachment
			attachments[1].format = m_Swapchain->GetSpecification().DepthFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// attachments reference
			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// subpass
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorReference;
			subpass.pDepthStencilAttachment = &depthReference;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
			subpass.pResolveAttachments = nullptr;

			// subpass dependencies
			std::array<VkSubpassDependency, 2> dependencies = {};

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[0].srcSubpass = 0;
			dependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = (u32)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = (u32)dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			VK_CHECK_RESULT(vkCreateRenderPass(m_Device->GetSpecification().Device, &renderPassCI, nullptr, &m_Specification.RenderPass));
		}

		// setup pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCI = {};
		pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCI.pNext = nullptr;
		VK_CHECK_RESULT(vkCreatePipelineCache(m_Device->GetSpecification().Device, &pipelineCacheCI, nullptr, &m_Specification.PipelineCache));

		SetupFramebuffer();
	}

	void VKRenderer::CreateSyncSystem()
	{
		m_Specification.Syncronization.WaitFences.resize(m_Specification.Syncronization.RenderAhead);
		m_Specification.Syncronization.RenderCompleteSemaphores.resize(m_Specification.Syncronization.RenderAhead);
		m_Specification.Syncronization.PresentationCompleteSemaphore.resize(m_Specification.Syncronization.RenderAhead);

		for (size_t i = 0; i < m_Specification.Syncronization.RenderAhead; i++)
		{
			VkFenceCreateInfo fenceCI = {};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.pNext = nullptr;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VK_CHECK_RESULT(vkCreateFence(m_Device->GetSpecification().Device, &fenceCI, nullptr, &m_Specification.Syncronization.WaitFences[i]));
			
			VkSemaphoreCreateInfo semaphore1CI = {};
			semaphore1CI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore1CI.pNext = nullptr;
			semaphore1CI.flags = 0;
			VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetSpecification().Device, &semaphore1CI, nullptr, &m_Specification.Syncronization.RenderCompleteSemaphores[i]));
			
			VkSemaphoreCreateInfo semaphore2CI = {};
			semaphore2CI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore2CI.pNext = nullptr;
			semaphore2CI.flags = 0;
			VK_CHECK_RESULT(vkCreateSemaphore(m_Device->GetSpecification().Device, &semaphore2CI, nullptr, &m_Specification.Syncronization.PresentationCompleteSemaphore[i]));
		}
	}

	void VKRenderer::CreateCommandBuffers()
	{
		m_Specification.CommandBuffers.resize(m_Swapchain->GetSpecification().ImageCount);

		VkCommandBufferAllocateInfo commandBufferAI = {};
		commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAI.pNext = nullptr;
		commandBufferAI.commandPool = m_Specification.CommandPool;
		commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAI.commandBufferCount = (u32)m_Specification.CommandBuffers.size();
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device->GetSpecification().Device, &commandBufferAI, m_Specification.CommandBuffers.data()));
	}

	void VKRenderer::SetupFramebuffer()
	{
		// msaa
		if (m_Settings.Multisampling)
		{
			// checks if device supports sample count for color and depth framebuffer

			bool supports = true;
			supports &= m_Device->GetSpecification().Properties.limits.framebufferColorSampleCounts >= (VkSampleCountFlags)m_Settings.Samples;
			supports &= m_Device->GetSpecification().Properties.limits.framebufferDepthSampleCounts >= (VkSampleCountFlags)m_Settings.Samples;

			LOG_ASSERT(supports == true, "Device does not support the current multisampling");

			// color target
			VkImageCreateInfo imageCI = {};
			imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.pNext = nullptr;
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			imageCI.format = m_Swapchain->GetSpecification().ColorFormat;
			imageCI.extent.width = m_Swapchain->GetSpecification().Extent.width;
			imageCI.extent.height = m_Swapchain->GetSpecification().Extent.height;
			imageCI.extent.depth = 1;
			imageCI.mipLevels = 1;
			imageCI.arrayLayers = 1;
			imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCI.samples = m_Settings.Samples;
			imageCI.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCI, nullptr, &m_Specification.MultiSampleTarget.Color.image));

			VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Color.image, &memoryRequirements);

			VkBool32 lazyMemory;
			VkMemoryAllocateInfo memoryAI = {};
			memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAI.pNext = nullptr;
			memoryAI.allocationSize = memoryRequirements.size;
			memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, &lazyMemory);

			if (!lazyMemory)
			{
				memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}

			VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &m_Specification.MultiSampleTarget.Color.memory));
			vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Color.image, m_Specification.MultiSampleTarget.Color.memory, 0);

			// image view for msaa color target
			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCI.pNext = nullptr;
			imageViewCI.image = m_Specification.MultiSampleTarget.Color.image;
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = m_Swapchain->GetSpecification().ColorFormat;
			imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCI.subresourceRange.levelCount = 1;
			imageViewCI.subresourceRange.layerCount = 1;
			VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.MultiSampleTarget.Color.view));

			// depth target
			imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.pNext = nullptr;
			imageCI.imageType = VK_IMAGE_TYPE_2D;
			imageCI.format = m_Swapchain->GetSpecification().DepthFormat;
			imageCI.extent.width = m_Swapchain->GetSpecification().Extent.width;
			imageCI.extent.height = m_Swapchain->GetSpecification().Extent.height;
			imageCI.extent.depth = 1;
			imageCI.mipLevels = 1;
			imageCI.arrayLayers = 1;
			imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCI.samples = m_Settings.Samples;
			imageCI.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCI, nullptr, &m_Specification.MultiSampleTarget.Depth.image));

			vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Depth.image, &memoryRequirements);

			memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAI.pNext = nullptr;
			memoryAI.allocationSize = memoryRequirements.size;
			memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, &lazyMemory);

			if (!lazyMemory)
			{
				memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			}

			VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &m_Specification.MultiSampleTarget.Depth.memory));
			vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.MultiSampleTarget.Depth.image, m_Specification.MultiSampleTarget.Depth.memory, 0);
			
			// image view for msaa depth target
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCI.pNext = nullptr;
			imageViewCI.image = m_Specification.MultiSampleTarget.Depth.image;
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = m_Swapchain->GetSpecification().DepthFormat;
			imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			imageViewCI.subresourceRange.levelCount = 1;
			imageViewCI.subresourceRange.layerCount = 1;
			VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.MultiSampleTarget.Depth.view));
		}

		// depth attachment is the same for multisampled and not-multisampled framebuffers
		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.pNext = nullptr;
		imageCI.flags = 0;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = m_Swapchain->GetSpecification().DepthFormat;
		imageCI.extent.width = m_Swapchain->GetSpecification().Extent.width;
		imageCI.extent.height = m_Swapchain->GetSpecification().Extent.height;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		VK_CHECK_RESULT(vkCreateImage(m_Device->GetSpecification().Device, &imageCI, nullptr, &m_Specification.DepthStencil.image));
		
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(m_Device->GetSpecification().Device, m_Specification.DepthStencil.image, &memoryRequirements);

		VkMemoryAllocateInfo memoryAI = {};
		memoryAI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAI.pNext = nullptr;
		memoryAI.allocationSize = memoryRequirements.size;
		memoryAI.memoryTypeIndex = m_Device->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_Device->GetSpecification().Device, &memoryAI, nullptr, &m_Specification.DepthStencil.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_Device->GetSpecification().Device, m_Specification.DepthStencil.image, m_Specification.DepthStencil.memory, 0));

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.pNext = nullptr;
		imageViewCI.flags = 0;
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = m_Swapchain->GetSpecification().DepthFormat;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.image = m_Specification.DepthStencil.image;
		VK_CHECK_RESULT(vkCreateImageView(m_Device->GetSpecification().Device, &imageViewCI, nullptr, &m_Specification.DepthStencil.view));

		// attachments
		VkImageView attachments[4] = {};

		if (m_Settings.Multisampling)
		{
			attachments[0] = m_Specification.MultiSampleTarget.Color.view;
			attachments[2] = m_Specification.MultiSampleTarget.Depth.view;
			attachments[3] = m_Specification.DepthStencil.view;
		}

		else
		{
			attachments[1] = m_Specification.DepthStencil.view;
		}
		
		VkFramebufferCreateInfo framebufferCI = {};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.pNext = nullptr;
		framebufferCI.flags = 0;
		framebufferCI.renderPass = m_Specification.RenderPass;
		framebufferCI.attachmentCount = m_Settings.Multisampling ? 4 : 2; LOG_WARNING("Check this attachment at a latter stage");
		framebufferCI.pAttachments = attachments;
		framebufferCI.width = m_Swapchain->GetSpecification().Width;
		framebufferCI.height = m_Swapchain->GetSpecification().Height;
		framebufferCI.layers = 1;

		// create a framebuffer for every image in the swapchain (move this to swapchain?)
		m_Specification.FrameBuffers.resize(m_Swapchain->GetSpecification().ImageCount);
		for (u32 i = 0; i < m_Specification.FrameBuffers.size(); i++)
		{
			if (m_Settings.Multisampling)
			{
				attachments[1] = m_Swapchain->GetSpecification().Buffers[i].View;
			}

			else
			{
				attachments[0] = m_Swapchain->GetSpecification().Buffers[i].View;
			}

			VK_CHECK_RESULT(vkCreateFramebuffer(m_Device->GetSpecification().Device, &framebufferCI, nullptr, &m_Specification.FrameBuffers[i]));
		}
	}
}