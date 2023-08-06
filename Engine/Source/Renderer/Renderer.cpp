#include "Renderer.h"

#include "Vulkan/VKDevice.h"
#include "Vulkan/VKInstance.h"
#include "Vulkan/VKUtility.h"

#include "Core/UI.h"
#include "Platform/Window.h"
#include "Util/Logger.h"

#include <array>

namespace Cosmos
{
	std::shared_ptr<Renderer> Renderer::Create(std::shared_ptr<Window>& window)
	{
		return std::make_shared<Renderer>(window);
	}

	Renderer::Renderer(std::shared_ptr<Window>& window)
		: mWindow(window)
	{
		mInstance = VKInstance::Create("Cosmos Application", "Cosmos", true);
		mDevice = VKDevice::Create(mWindow, mInstance);
		mSwapchain = VKSwapchain::Create(mWindow, mInstance, mDevice);

		CreateGlobalStates();

		mUI = UI::Create(mWindow, mInstance, mDevice, mSwapchain);

		mUBO = UBO::Create(mDevice, mRenderPass, mPipelineCache);
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(mDevice->Device());

		vkDestroyRenderPass(mDevice->Device(), mRenderPass, nullptr);
		vkDestroyPipelineCache(mDevice->Device(), mPipelineCache, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(mDevice->Device(), mInFlightFences[i], nullptr);
			vkDestroySemaphore(mDevice->Device(), mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice->Device(), mImageAvailableSemaphores[i], nullptr);
		}
	}

	void Renderer::Update()
	{
		// updating UI
		mUI->Update();

		uint32_t imageIndex;
		VkResult res;

		// acquire next image in the swapchain
		{
			vkWaitForFences(mDevice->Device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

			res = vkAcquireNextImageKHR(mDevice->Device(), mSwapchain->Swapchain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

			if (res == VK_ERROR_OUT_OF_DATE_KHR)
			{
				mSwapchain->Recreate(mRenderPass, mMSAACount);
				return;
			}

			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			{
				LOG_ASSERT(false, "Failed to acquired next swapchain image");
			}

			vkResetFences(mDevice->Device(), 1, &mInFlightFences[mCurrentFrame]);
		}

		ManageRenderPasses(imageIndex);

		VkSwapchainKHR swapChains[] = { mSwapchain->Swapchain() };
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		// submits to graphics queue
		{
			// grab all commandbuffers used into a single queue submit
			std::array<VkCommandBuffer, 2> submitCommandBuffers =
			{
				mDevice->CommandBuffers()[mCurrentFrame],
				mUI->CommandBuffers()[mCurrentFrame]
			};

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = nullptr;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
			submitInfo.pCommandBuffers = submitCommandBuffers.data();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
			LOG_ASSERT(vkQueueSubmit(mDevice->GraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) == VK_SUCCESS, "Failed to submit draw command");
		}

		// presents the image
		{
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			res = vkQueuePresentKHR(mDevice->PresentQueue(), &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || mWindow->ShouldResizeWindow())
			{
				mWindow->HintResizeWindow(false);
				mSwapchain->Recreate(mRenderPass, mMSAACount);

				mUI->SetImageCount(mSwapchain->ImageCount());
				mUI->Resize();
			}

			else if (res != VK_SUCCESS)
			{
				LOG_ASSERT(false, "Failed to present swapchain image");
			}
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	void Renderer::ManageRenderPasses(uint32_t& imageIndex)
	{
		// color and depth render pass
		{
			vkResetCommandBuffer(mDevice->CommandBuffers()[mCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			LOG_ASSERT(vkBeginCommandBuffer(mDevice->CommandBuffers()[mCurrentFrame], &cmdBeginInfo) == VK_SUCCESS, "Failed to begin command buffer recording");

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { {0.5f, 0.0f, 0.5f, 1.0f} };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = mRenderPass;
			renderPassBeginInfo.framebuffer = mSwapchain->Framebuffers()[imageIndex];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->Extent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(mDevice->CommandBuffers()[mCurrentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->Extent().width;
			viewport.height = (float)mSwapchain->Extent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(mDevice->CommandBuffers()[mCurrentFrame], 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->Extent();
			vkCmdSetScissor(mDevice->CommandBuffers()[mCurrentFrame], 0, 1, &scissor);

			vkCmdEndRenderPass(mDevice->CommandBuffers()[mCurrentFrame]);

			// end command buffer
			LOG_ASSERT(vkEndCommandBuffer(mDevice->CommandBuffers()[mCurrentFrame]) == VK_SUCCESS, "Failed to end command buffer recording");
		}

		// user interface
		{
			vkResetCommandBuffer(mUI->CommandBuffers()[mCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			LOG_ASSERT(vkBeginCommandBuffer(mUI->CommandBuffers()[mCurrentFrame], &cmdBeginInfo) == VK_SUCCESS, "Failed to begin command buffer recording");

			VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = mUI->RenderPass();
			renderPassBeginInfo.framebuffer = mUI->Framebuffers()[imageIndex];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->Extent();
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(mUI->CommandBuffers()[mCurrentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			mUI->Draw(mUI->CommandBuffers()[mCurrentFrame]);

			vkCmdEndRenderPass(mUI->CommandBuffers()[mCurrentFrame]);

			LOG_ASSERT(vkEndCommandBuffer(mUI->CommandBuffers()[mCurrentFrame]) == VK_SUCCESS, "Failed to end command buffer recording");
		}
	}

	void Renderer::CreateGlobalStates()
	{
		mMSAACount = mDevice->GetMaxUsableSamples();

		// render pass
		{
			// attachments descriptions
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = mSwapchain->SurfaceFormat().format;
			colorAttachment.samples = mMSAACount;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription colorAttachmentResolve = {};
			colorAttachmentResolve.format = mSwapchain->SurfaceFormat().format;
			colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// finalLayout should not be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as ui is a post render pass that will present

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = FindDepthFormat(mDevice);
			depthAttachment.samples = mMSAACount;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// attachments references
			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentResolveRef = {};
			colorAttachmentResolveRef.attachment = 2;
			colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// subpass
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;
			subpass.pResolveAttachments = &colorAttachmentResolveRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = (uint32_t)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = 1;
			renderPassCI.pDependencies = &dependency;
			LOG_ASSERT(vkCreateRenderPass(mDevice->Device(), &renderPassCI, nullptr, &mRenderPass) == VK_SUCCESS, "Failed to create render pass");

			mSwapchain->CreateFramebuffers(mRenderPass, mMSAACount);
		}

		// sync objects
		{
			mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreCI = {};
			semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCI.pNext = nullptr;
			semaphoreCI.flags = 0;

			VkFenceCreateInfo fenceCI = {};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.pNext = nullptr;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				LOG_ASSERT(vkCreateSemaphore(mDevice->Device(), &semaphoreCI, nullptr, &mImageAvailableSemaphores[i]) == VK_SUCCESS, "Failed to create image available semaphore");
				LOG_ASSERT(vkCreateSemaphore(mDevice->Device(), &semaphoreCI, nullptr, &mRenderFinishedSemaphores[i]) == VK_SUCCESS, "Failed to create render finished semaphore");
				LOG_ASSERT(vkCreateFence(mDevice->Device(), &fenceCI, nullptr, &mInFlightFences[i]) == VK_SUCCESS, "Failed to create in flight fence");
			}
		}

		// pipeline cache
		{
			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			pipelineCacheCI.pNext = nullptr;
			pipelineCacheCI.flags = 0;
			LOG_ASSERT(vkCreatePipelineCache(mDevice->Device(), &pipelineCacheCI, nullptr, &mPipelineCache) == VK_SUCCESS, "Failed to create pipeline cache");
		}
	}
}