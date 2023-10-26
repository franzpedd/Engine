#include "Renderer.h"

#include "Core/Scene.h"
#include "Platform/Window.h"
#include "UI/UICore.h"
#include "Util/Logger.h"

#include <array>

namespace Cosmos
{
	std::shared_ptr<Renderer> Renderer::Create(std::shared_ptr<Window>& window, std::shared_ptr<Scene>& scene)
	{
		return std::make_shared<Renderer>(window, scene);
	}

	Renderer::Renderer(std::shared_ptr<Window>& window, std::shared_ptr<Scene>& scene)
		: mWindow(window), mScene(scene)
	{
		mInstance = VKInstance::Create("Cosmos Application", "Cosmos", true);
		mDevice = VKDevice::Create(mWindow, mInstance);
		mSwapchain = VKSwapchain::Create(mWindow, mInstance, mDevice);

		CreateGlobalStates();
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(mDevice->Device());

		vkDestroyPipelineCache(mDevice->Device(), mPipelineCache, nullptr);

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(mDevice->Device(), mInFlightFences[i], nullptr);
			vkDestroySemaphore(mDevice->Device(), mRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(mDevice->Device(), mImageAvailableSemaphores[i], nullptr);
		}

		mPipelineLibrary.DestroyAllPipelines();
	}

	void Renderer::OnUpdate()
	{
		Render();
	}

	void Renderer::Render()
	{
		VkResult res;

		// acquire next image in the swapchain
		{
			vkWaitForFences(mDevice->Device(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

			res = vkAcquireNextImageKHR(mDevice->Device(), mSwapchain->Swapchain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);

			if (res == VK_ERROR_OUT_OF_DATE_KHR)
			{
				mSwapchain->Recreate();
				return;
			}

			else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
			{
				LOG_ASSERT(false, "Failed to acquired next swapchain image");
			}

			vkResetFences(mDevice->Device(), 1, &mInFlightFences[mCurrentFrame]);
		}

		ManageRenderPasses(mImageIndex);

		VkSwapchainKHR swapChains[] = { mSwapchain->Swapchain() };
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		// submits to graphics queue
		{
			// grab all commandbuffers used into a single queue submit
			std::array<VkCommandBuffer, 3> submitCommandBuffers =
			{
				mCommander.Access()[0]->commandBuffers[mCurrentFrame], // swapchain
				mCommander.Access()[2]->commandBuffers[mCurrentFrame], // viewport
				mCommander.Access()[1]->commandBuffers[mCurrentFrame]  // imgui
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
			VK_ASSERT(vkQueueSubmit(mDevice->GraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]), "Failed to submit draw command");
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

			res = vkQueuePresentKHR(mDevice->PresentQueue(), &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || mWindow->ShouldResizeWindow())
			{
				mWindow->HintResizeWindow(false);
				mSwapchain->Recreate();

				mUI->SetImageCount(mSwapchain->ImageCount());
				mUI->OnResize();
			}

			else if (res != VK_SUCCESS)
			{
				LOG_ASSERT(false, "Failed to present swapchain image");
			}
		}

		mCurrentFrame = (mCurrentFrame + 1) % RENDERER_MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::ManageRenderPasses(uint32_t& imageIndex)
	{
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// color and depth render pass
		{
			VkCommandBuffer& cmdBuffer = mCommander.Access()[0]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander.Access()[0]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander.Access()[0]->renderPass;

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
			renderPassBeginInfo.renderArea.extent = mSwapchain->Extent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->Extent().width;
			viewport.height = (float)mSwapchain->Extent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->Extent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render scene

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// viewport
		{
			VkCommandBuffer& cmdBuffer = mCommander.Access()[2]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander.Access()[2]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander.Access()[2]->renderPass;

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
			renderPassBeginInfo.renderArea.extent = mSwapchain->Extent();
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// set frame commandbuffer viewport
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)mSwapchain->Extent().width;
			viewport.height = (float)mSwapchain->Extent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

			// set frame commandbuffer scissor
			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = mSwapchain->Extent();
			vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

			// render scene
			mScene->OnDraw();

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// user interface
		{
			VkCommandBuffer& cmdBuffer = mCommander.Access()[1]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander.Access()[1]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander.Access()[1]->renderPass;

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
			renderPassBeginInfo.renderArea.extent = mSwapchain->Extent();
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			mUI->Draw(cmdBuffer);

			vkCmdEndRenderPass(cmdBuffer);

			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}
	}

	void Renderer::CreateGlobalStates()
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
				VK_ASSERT(vkCreateSemaphore(mDevice->Device(), &semaphoreCI, nullptr, &mImageAvailableSemaphores[i]), "Failed to create image available semaphore");
				VK_ASSERT(vkCreateSemaphore(mDevice->Device(), &semaphoreCI, nullptr, &mRenderFinishedSemaphores[i]), "Failed to create render finished semaphore");
				VK_ASSERT(vkCreateFence(mDevice->Device(), &fenceCI, nullptr, &mInFlightFences[i]), "Failed to create in flight fence");
			}
		}

		// pipeline cache
		{
			VkPipelineCacheCreateInfo pipelineCacheCI = {};
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			pipelineCacheCI.pNext = nullptr;
			pipelineCacheCI.flags = 0;
			VK_ASSERT(vkCreatePipelineCache(mDevice->Device(), &pipelineCacheCI, nullptr, &mPipelineCache), "Failed to create pipeline cache");
		}
	}
}