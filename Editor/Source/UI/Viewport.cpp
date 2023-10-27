#include "Viewport.h"

#include <array>

namespace Cosmos
{
	Viewport::Viewport(std::shared_ptr<UICore>& ui, std::shared_ptr<Renderer>& renderer, Camera& camera)
		: mUI(ui), mRenderer(renderer), mCamera(camera)

	{
		mCommandEntry = CommandEntry::Create(renderer->BackendDevice()->Device(), "Viewport");
		Commander::Get().Add(mCommandEntry);

		// set viewport renderpass to main renderpass
		auto& mainCommander = mRenderer->GetCommander().Access()[2];
		mRenderer->GetCommander().AccessMainCommandEntry() = mainCommander;

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Rework Commander class and main RenderPass usage");

		mSurfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
		mDepthFormat = FindDepthFormat(mRenderer->BackendDevice());

		// render pass
		{
			std::array<VkAttachmentDescription, 2> attachments = {};

			// color attachment
			attachments[0].format = mSurfaceFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// depth attachment
			attachments[1].format = mDepthFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDescription = {};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;
			subpassDescription.pDepthStencilAttachment = &depthReference;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;
			subpassDescription.pResolveAttachments = nullptr;

			// Subpass dependencies for layout transitions
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
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = (uint32_t)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDescription;
			renderPassCI.dependencyCount = (uint32_t)dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			VK_ASSERT(vkCreateRenderPass(mRenderer->BackendDevice()->Device(), &renderPassCI, nullptr, &mCommandEntry->renderPass), "Failed to create renderpass");
		}

		// command pool
		{
			QueueFamilyIndices indices = mRenderer->BackendDevice()->FindQueueFamilies(mRenderer->BackendDevice()->PhysicalDevice(), mRenderer->BackendDevice()->Surface());

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_ASSERT(vkCreateCommandPool(mRenderer->BackendDevice()->Device(), &cmdPoolInfo, nullptr, &mCommandEntry->commandPool), "Failed to create command pool");
		}

		// command buffers
		{
			mCommandEntry->commandBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = mCommandEntry->commandPool;
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)mCommandEntry->commandBuffers.size();
			VK_ASSERT(vkAllocateCommandBuffers(mRenderer->BackendDevice()->Device(), &cmdBufferAllocInfo, mCommandEntry->commandBuffers.data()), "Failed to allocate command buffers");
		}

		// sampler
		{
			VkSamplerCreateInfo samplerCI = {};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.anisotropyEnable = VK_TRUE;
			samplerCI.maxAnisotropy = mRenderer->BackendDevice()->Properties().limits.maxSamplerAnisotropy;
			samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerCI.unnormalizedCoordinates = VK_FALSE;
			samplerCI.compareEnable = VK_FALSE;
			samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			VK_ASSERT(vkCreateSampler(mRenderer->BackendDevice()->Device(), &samplerCI, nullptr, &mSampler), "Failed to create sampler");
		}

		CreateResources();
	}

	Viewport::~Viewport()
	{
		vkDestroySampler(mRenderer->BackendDevice()->Device(), mSampler, nullptr);

		vkDestroyImageView(mRenderer->BackendDevice()->Device(), mDepthView, nullptr);
		vkFreeMemory(mRenderer->BackendDevice()->Device(), mDepthMemory, nullptr);
		vkDestroyImage(mRenderer->BackendDevice()->Device(), mDepthImage, nullptr);

		for (size_t i = 0; i < mRenderer->BackendSwapchain()->Images().size(); i++)
		{
			vkDestroyImageView(mRenderer->BackendDevice()->Device(), mImageViews[i], nullptr);
			vkFreeMemory(mRenderer->BackendDevice()->Device(), mImageMemories[i], nullptr);
			vkDestroyImage(mRenderer->BackendDevice()->Device(), mImages[i], nullptr);
		}

		mCommandEntry->Destroy();
	}

	void Viewport::OnUpdate()
	{
		ImGui::Begin("Scene viewport");
		ImGui::Image(mDescriptorSets[mRenderer->CurrentFrame()], ImGui::GetContentRegionAvail());

		// updating aspect ratio for the docking
		mCurrentSize = ImGui::GetWindowSize();
		mCamera.SetAspectRatio(mCurrentSize.x / mCurrentSize.y);

		ImGui::End();
	}

	void Viewport::OnResize()
	{
		vkDestroyImageView(mRenderer->BackendDevice()->Device(), mDepthView, nullptr);
		vkFreeMemory(mRenderer->BackendDevice()->Device(), mDepthMemory, nullptr);
		vkDestroyImage(mRenderer->BackendDevice()->Device(), mDepthImage, nullptr);

		for (size_t i = 0; i < mRenderer->BackendSwapchain()->Images().size(); i++)
		{
			vkDestroyImageView(mRenderer->BackendDevice()->Device(), mImageViews[i], nullptr);
			vkFreeMemory(mRenderer->BackendDevice()->Device(), mImageMemories[i], nullptr);
			vkDestroyImage(mRenderer->BackendDevice()->Device(), mImages[i], nullptr);
		}

		for (auto& framebuffer : mCommandEntry->framebuffers)
		{
			vkDestroyFramebuffer(mRenderer->BackendDevice()->Device(), framebuffer, nullptr);
		}

		CreateResources();
	}

	void Viewport::CreateResources()
	{
		size_t size = mRenderer->BackendSwapchain()->Images().size();

		// depth buffer
		{
			CreateImage
			(
				mRenderer->BackendDevice(),
				mRenderer->BackendSwapchain()->Extent().width,
				mRenderer->BackendSwapchain()->Extent().height,
				1,
				VK_SAMPLE_COUNT_1_BIT,
				mDepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = CreateImageView(mRenderer->BackendDevice(), mDepthImage, mDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		// images
		{
			mImages.resize(size);
			mImageMemories.resize(size);
			mImageViews.resize(size);
			mDescriptorSets.resize(size);
			mCommandEntry->framebuffers.resize(size);

			for (size_t i = 0; i < size; i++)
			{
				CreateImage
				(
					mRenderer->BackendDevice(),
					mRenderer->BackendSwapchain()->Extent().width,
					mRenderer->BackendSwapchain()->Extent().height,
					1,
					VK_SAMPLE_COUNT_1_BIT,
					mSurfaceFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					mImages[i],
					mImageMemories[i]
				);

				VkCommandBuffer command = BeginSingleTimeCommand(mRenderer->BackendDevice(), mCommandEntry->commandPool);

				InsertImageMemoryBarrier
				(
					command,
					mImages[i],
					VK_ACCESS_TRANSFER_READ_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				);

				EndSingleTimeCommand(mRenderer->BackendDevice(), mCommandEntry->commandPool, command);

				mImageViews[i] = CreateImageView(mRenderer->BackendDevice(), mImages[i], mSurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);
				mDescriptorSets[i] = AddTexture(mSampler, mImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::array<VkImageView, 2> attachments = { mImageViews[i], mDepthView };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = mCommandEntry->renderPass;
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = mRenderer->BackendSwapchain()->Extent().width;
				framebufferCI.height = mRenderer->BackendSwapchain()->Extent().height;
				framebufferCI.layers = 1;
				VK_ASSERT(vkCreateFramebuffer(mRenderer->BackendDevice()->Device(), &framebufferCI, nullptr, &mCommandEntry->framebuffers[i]), "Failed to create framebuffer");
			}
		}
	}
}