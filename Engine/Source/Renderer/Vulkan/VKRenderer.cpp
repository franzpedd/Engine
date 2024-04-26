#include "epch.h"
#include "VKRenderer.h"

#include "VKInitializers.h"
#include "VKShader.h"

#include "Core/Application.h"
#include "Core/Scene.h"
#include "Event/WindowEvent.h"
#include "UI/GUI.h"
#include "Util/FileSystem.h"

namespace Cosmos
{
	VKRenderer::VKRenderer()
	{
		mInstance = VKInstance::Create("Cosmos Application", "Cosmos", true);
		mDevice = VKDevice::Create(mInstance);
		mCommander = CreateShared<VKCommander>();
		mSwapchain = VKSwapchain::Create(mInstance, mDevice);

		CreateResources();
		CreateGlobalStates();
	}

	VKRenderer::~VKRenderer()
	{
		vkDeviceWaitIdle(mDevice->GetDevice());

		vkDestroyPipelineCache(mDevice->GetDevice(), mPipelineCache, nullptr);

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(mDevice->GetDevice(), mInFlightFences[i], nullptr);
			vkDestroySemaphore(mDevice->GetDevice(), mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice->GetDevice(), mImageAvailableSemaphores[i], nullptr);
		}

		for (auto& pipeline : mPipelines)
			vkDestroyPipeline(mDevice->GetDevice(), pipeline.second, nullptr);

		for (auto& layout : mPipelineLayouts)
			vkDestroyPipelineLayout(mDevice->GetDevice(), layout.second, nullptr);

		for (auto& descriptorSetLayout : mDescriptorSetLayouts)
			vkDestroyDescriptorSetLayout(mDevice->GetDevice(), descriptorSetLayout.second, nullptr);
	}

	VkPipelineCache& VKRenderer::PipelineCache()
	{
		return mPipelineCache;
	}

	VkPipeline VKRenderer::GetPipeline(std::string nameid)
	{
		if (mPipelines[nameid])
			return mPipelines[nameid];

		LOG_TO_TERMINAL(Logger::Error, "No pipeline with nameid %s exists", nameid.c_str());
		return nullptr;
	}

	VkDescriptorSetLayout VKRenderer::GetDescriptorSetLayout(std::string nameid)
	{
		if (mDescriptorSetLayouts[nameid])
			return mDescriptorSetLayouts[nameid];

		LOG_TO_TERMINAL(Logger::Error, "No descriptor set layout with nameid %s exists", nameid.c_str());
		return nullptr;
	}

	VkPipelineLayout VKRenderer::GetPipelineLayout(std::string nameid)
	{
		if (mPipelineLayouts[nameid])
			return mPipelineLayouts[nameid];

		LOG_TO_TERMINAL(Logger::Error, "No pipeline layout with nameid %s exists", nameid.c_str());
		return nullptr;
	}

	uint32_t VKRenderer::CurrentFrame()
	{
		return mCurrentFrame;
	}

	uint32_t VKRenderer::ImageIndex()
	{
		return mImageIndex;
	}

	void VKRenderer::OnUpdate()
	{
		PROFILER_FUNCTION();

		VkResult res;

		// acquire next image in the swapchain
		{
			PROFILER_SCOPE("Swapchain Next Image");
			vkWaitForFences(mDevice->GetDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

			res = vkAcquireNextImageKHR(mDevice->GetDevice(), mSwapchain->GetSwapchain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);

			if (res == VK_ERROR_OUT_OF_DATE_KHR)
			{
				mSwapchain->Recreate();
				return;
			}

			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			{
				LOG_ASSERT(false, "Failed to acquired next swapchain image");
			}

			vkResetFences(mDevice->GetDevice(), 1, &mInFlightFences[mCurrentFrame]);
		}

		ManageRenderPasses(mImageIndex);

		VkSwapchainKHR swapChains[] = { mSwapchain->GetSwapchain() };
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		// submits to graphics queue
		{
			// 
			std::vector<VkCommandBuffer> submitCommandBuffers = { mCommander->GetEntriesRef()["Swapchain"]->commandBuffers[mCurrentFrame] };

			if (mCommander->Exists("Viewport"))
			{
				submitCommandBuffers.push_back(mCommander->GetEntriesRef()["Viewport"]->commandBuffers[mCurrentFrame]);
			}

			if (mCommander->Exists("ImGui"))
			{
				submitCommandBuffers.push_back(mCommander->GetEntriesRef()["ImGui"]->commandBuffers[mCurrentFrame]);
			}

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
			VK_ASSERT(vkQueueSubmit(mDevice->GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]), "Failed to submit draw command");
		}

		// presents the image
		{
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &mImageIndex;

			res = vkQueuePresentKHR(mDevice->GetPresentQueue(), &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || Application::GetInstance()->GetWindow()->ShouldResizeWindow())
			{
				Application::GetInstance()->GetWindow()->HintResizeWindow(false);
				mSwapchain->Recreate();

				Application::GetInstance()->GetCamera()->SetAspectRatio(Application::GetInstance()->GetWindow()->GetAspectRatio());
				Application::GetInstance()->GetGUI()->SetImageCount(mSwapchain->GetImageCount());

				int32_t width = (int32_t)mSwapchain->GetExtent().width;
				int32_t height = (int32_t)mSwapchain->GetExtent().height;

				Shared<WindowResizeEvent> event = CreateShared<WindowResizeEvent>(width, height);
				Application::GetInstance()->OnEvent(event);
			}

			else if (res != VK_SUCCESS)
			{
				LOG_ASSERT(false, "Failed to present swapchain image");
			}
		}

		mCurrentFrame = (mCurrentFrame + 1) % RENDERER_MAX_FRAMES_IN_FLIGHT;
	}

	void VKRenderer::ManageRenderPasses(uint32_t& imageIndex)
	{
		PROFILER_FUNCTION();

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// color and depth render pass
		if(mCommander->Exists("Swapchain"))
		{
			VkCommandBuffer& cmdBuffer = mCommander->GetEntriesRef()["Swapchain"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntriesRef()["Swapchain"]->frameBuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntriesRef()["Swapchain"]->renderPass;

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo), "Failed to begin command buffer recording");

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->GetExtent().width;
			viewport.height = (float)mSwapchain->GetExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->GetExtent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render scene, only if not on viewport
			if (!mCommander->Exists("Viewport"))
			{
				Application::GetInstance()->GetActiveScene()->OnRender();
			}

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// viewport
		if(mCommander->Exists("Viewport"))
		{
			VkCommandBuffer& cmdBuffer = mCommander->GetEntriesRef()["Viewport"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntriesRef()["Viewport"]->frameBuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntriesRef()["Viewport"]->renderPass;

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo), "Failed to begin command buffer recording");

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->GetExtent().width;
			viewport.height = (float)mSwapchain->GetExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->GetExtent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render scene and special widgets
			Application::GetInstance()->GetActiveScene()->OnRender();

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// user interface
		if(mCommander->Exists("ImGui"))
		{
			VkCommandBuffer& cmdBuffer = mCommander->GetEntriesRef()["ImGui"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntriesRef()["ImGui"]->frameBuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntriesRef()["ImGui"]->renderPass;

			vkResetCommandBuffer(cmdBuffer, /*VkCommandBufferResetFlagBits*/ 0);

			VkCommandBufferBeginInfo cmdBeginInfo = {};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.pNext = nullptr;
			cmdBeginInfo.flags = 0;
			VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo), "Failed to begin command buffer recording");

			VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.framebuffer = frameBuffer;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent();
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			Application::GetInstance()->GetGUI()->Draw(cmdBuffer);

			vkCmdEndRenderPass(cmdBuffer);

			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}
	}

	void VKRenderer::CreateResources()
	{
		// sync objects
		{
			mImageAvailableSemaphores.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mRenderFinishedSemaphores.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mInFlightFences.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreCI = {};
			semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCI.pNext = nullptr;
			semaphoreCI.flags = 0;

			VkFenceCreateInfo fenceCI = {};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.pNext = nullptr;
			fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
			{
				VK_ASSERT(vkCreateSemaphore(mDevice->GetDevice(), &semaphoreCI, nullptr, &mImageAvailableSemaphores[i]), "Failed to create image available semaphore");
				VK_ASSERT(vkCreateSemaphore(mDevice->GetDevice(), &semaphoreCI, nullptr, &mRenderFinishedSemaphores[i]), "Failed to create render finished semaphore");
				VK_ASSERT(vkCreateFence(mDevice->GetDevice(), &fenceCI, nullptr, &mInFlightFences[i]), "Failed to create in flight fence");
			}
		}

		// pipeline cache
		{
			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			pipelineCacheCI.pNext = nullptr;
			pipelineCacheCI.flags = 0;
			VK_ASSERT(vkCreatePipelineCache(mDevice->GetDevice(), &pipelineCacheCI, nullptr, &mPipelineCache), "Failed to create pipeline cache");
		}
	}

	void VKRenderer::CreateGlobalStates()
	{
		// model global resources
		{
			mModelGlobalResource.Initialize(mDevice, mPipelineCache);
			mPipelines["Model"] = mModelGlobalResource.pipeline;
			mDescriptorSetLayouts["Model"] = mModelGlobalResource.descriptorSetLayout;
			mPipelineLayouts["Model"] = mModelGlobalResource.pipelineLayout;
		}
	}
}