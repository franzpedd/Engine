#include "epch.h"
#include "VKRenderer.h"

#include "VKInitializers.h"
#include "VKShader.h"
#include "Entity/Renderable/Vertex.h"

#include "Core/Scene.h"
#include "Platform/Window.h"
#include "UI/GUI.h"
#include "Util/FileSystem.h"

namespace Cosmos
{
	VKRenderer::VKRenderer(std::shared_ptr<Window>& window, Scene* scene)
		: mWindow(window), mScene(scene)
	{
		mInstance = VKInstance::Create("Cosmos Application", "Cosmos", true);
		mDevice = VKDevice::Create(mWindow, mInstance);
		mCommander = new Commander();

		LOG_TO_TERMINAL(Logger::Todo, "Move this to Swapchain");
		Commander::Get().Insert("Swapchain");
		Commander::Get().MakePrimary("Swapchain");
		Commander::Get().GetEntries()["Swapchain"]->msaa = mDevice->GetMSAA();

		mDevice->CreateCommandPool();
		mDevice->CreateCommandBuffers();

		mSwapchain = VKSwapchain::Create(mWindow, mInstance, mDevice);

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

	std::shared_ptr<Instance> VKRenderer::GetInstance()
	{
		return std::static_pointer_cast<Instance>(mInstance);
	}

	std::shared_ptr<Device> VKRenderer::GetDevice()
	{
		return std::static_pointer_cast<Device>(mDevice);
	}

	std::shared_ptr<Swapchain> VKRenderer::GetSwapchain()
	{
		return std::static_pointer_cast<Swapchain>(mSwapchain);
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

	void VKRenderer::Intialize()
	{
		mModelGlobalResource.Initialize(mDevice, mPipelineCache);
		mPipelines["Model"] = mModelGlobalResource.pipeline;
		mDescriptorSetLayouts["Model"] = mModelGlobalResource.descriptorSetLayout;
		mPipelineLayouts["Model"] = mModelGlobalResource.pipelineLayout;
	}

	void VKRenderer::OnTerminate()
	{
		mCommander->Erase("Swapchain", mDevice->GetDevice());
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
			std::vector<VkCommandBuffer> submitCommandBuffers = { mCommander->GetEntries()["Swapchain"]->commandBuffers[mCurrentFrame] };

			if (mCommander->Exists("Viewport"))
			{
				submitCommandBuffers.push_back(mCommander->GetEntries()["Viewport"]->commandBuffers[mCurrentFrame]);
			}

			if (mCommander->Exists("ImGui"))
			{
				submitCommandBuffers.push_back(mCommander->GetEntries()["ImGui"]->commandBuffers[mCurrentFrame]);
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

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || mWindow->ShouldResizeWindow())
			{
				mWindow->HintResizeWindow(false);
				mSwapchain->Recreate();

				mUI->SetImageCount(mSwapchain->GetImageCount());
				mUI->OnWindowResize();
			}

			else if (res != VK_SUCCESS)
			{
				LOG_ASSERT(false, "Failed to present swapchain image");
			}
		}

		mCurrentFrame = (mCurrentFrame + 1) % RENDERER_MAX_FRAMES_IN_FLIGHT;
	}

	void VKRenderer::ConnectUI(std::shared_ptr<GUI>& ui)
	{
		mUI = ui;
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
			VkCommandBuffer& cmdBuffer = mCommander->GetEntries()["Swapchain"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntries()["Swapchain"]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntries()["Swapchain"]->renderPass;

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
				mScene->OnRenderDraw();
			}

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// viewport
		if(mCommander->Exists("Viewport"))
		{
			VkCommandBuffer& cmdBuffer = mCommander->GetEntries()["Viewport"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntries()["Viewport"]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntries()["Viewport"]->renderPass;

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
			mScene->OnRenderDraw();

			vkCmdEndRenderPass(cmdBuffer);

			// end command buffer
			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}

		// user interface
		if(mCommander->Exists("ImGui"))
		{
			VkCommandBuffer& cmdBuffer = mCommander->GetEntries()["ImGui"]->commandBuffers[mCurrentFrame];
			VkFramebuffer& frameBuffer = mCommander->GetEntries()["ImGui"]->framebuffers[imageIndex];
			VkRenderPass& renderPass = mCommander->GetEntries()["ImGui"]->renderPass;

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

			mUI->Draw(cmdBuffer);

			vkCmdEndRenderPass(cmdBuffer);

			VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer recording");
		}
	}

	void VKRenderer::CreateGlobalStates()
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

	/*
	void VKRenderer::CreatePipelines()
	{
		// terrain pipeline
		/*
		{
			const std::string id = "Terrain";

			// descriptor sets
			// descriptor layout
			std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
			// tesselation
			bindings[0].binding = 0;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			bindings[0].descriptorCount = 1;
			bindings[0].pImmutableSamplers = nullptr;
			// height map
			bindings[1].binding = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[1].stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[1].descriptorCount = 1;
			bindings[1].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
			descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetLayoutCI.pNext = nullptr;
			descSetLayoutCI.flags = 0;
			descSetLayoutCI.pBindings = bindings.data();
			descSetLayoutCI.bindingCount = (uint32_t)bindings.size();
			VK_ASSERT(vkCreateDescriptorSetLayout(mDevice->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayouts[id]), "Failed to create descriptor set layout");



			// shaders
			std::shared_ptr<VKShader> vShader = VKShader::Create(mDevice, VKShader::Vertex, "Terrain.vert", util::GetAssetSubDir("Shaders/terrain.vert"));
			std::shared_ptr<VKShader> fShader = VKShader::Create(mDevice, VKShader::Fragment, "Terrain.frag", util::GetAssetSubDir("Shaders/terrain.frag"));
			std::shared_ptr<VKShader> tcShader = VKShader::Create(mDevice, VKShader::TessControl, "Terrain.tesc", util::GetAssetSubDir("Shaders/terrain.tesc"));
			std::shared_ptr<VKShader> teShader = VKShader::Create(mDevice, VKShader::TessEvaluation, "Terrain.tese", util::GetAssetSubDir("Shaders/terrain.tese"));

			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vShader->Stage(), fShader->Stage(), tcShader->Stage(), teShader->Stage() };
			const std::vector<VkVertexInputAttributeDescription> attributeDesc = Vertex::GetAttributeDescriptions();
			const std::vector<VkVertexInputBindingDescription> bindingDesc = Vertex::GetBindingDescription();
			
			VkPipelineVertexInputStateCreateInfo VISCI = vulkan::PipelineVertexInputStateCreateInfo(bindingDesc, attributeDesc);
			VkPipelineInputAssemblyStateCreateInfo IASCI = vulkan::PipelineInputStateCrateInfo(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, 0, VK_FALSE);
			VkPipelineTessellationStateCreateInfo TSCI = vulkan::PipelineTesselationStateCreateInfo(4);
			VkPipelineViewportStateCreateInfo VSCI = vulkan::PipelineViewportStateCreateInfo(1, 1);
			VkPipelineRasterizationStateCreateInfo RSCI = vulkan::PipelineRasterizationCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
			VkPipelineMultisampleStateCreateInfo MSCI = vulkan::PipelineMultisampleStateCreateInfo(Commander::Get().GetPrimary()->msaa);
			VkPipelineDepthStencilStateCreateInfo DSSCI = vulkan::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			VkPipelineColorBlendAttachmentState CBAS = vulkan::PipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
			VkPipelineColorBlendStateCreateInfo CBSCI = vulkan::PipelineColorBlendStateCreateInfo(1, &CBAS);
			VkPipelineDynamicStateCreateInfo DSCI = vulkan::PipelineDynamicStateCreateInfo(dynamicStates);

			VkGraphicsPipelineCreateInfo pipelineCI = {};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.pNext = nullptr;
			pipelineCI.flags = 0;
			pipelineCI.stageCount = (uint32_t)shaderStages.size();
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.pVertexInputState = &VISCI;
			pipelineCI.pInputAssemblyState = &IASCI;
			pipelineCI.pTessellationState = &TSCI;
			pipelineCI.pViewportState = &VSCI;
			pipelineCI.pRasterizationState = &RSCI;
			pipelineCI.pMultisampleState = &MSCI;
			pipelineCI.pDepthStencilState = &DSSCI;
			pipelineCI.pColorBlendState = &CBSCI;
			pipelineCI.pDynamicState = &DSCI;
			pipelineCI.layout = mPipelineLayouts[id];
			pipelineCI.renderPass = Commander::Get().GetPrimary()->renderPass;
			pipelineCI.subpass = 0;
			pipelineCI.basePipelineIndex = -1;
			pipelineCI.basePipelineHandle = VK_NULL_HANDLE;


			/*
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			uboLayoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutBinding albedoLayoutBinding = {};
			albedoLayoutBinding.binding = 1;
			albedoLayoutBinding.descriptorCount = 1;
			albedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			albedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			albedoLayoutBinding.pImmutableSamplers = nullptr;

			const std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, albedoLayoutBinding };

			VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
			descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetLayoutCI.pNext = nullptr;
			descSetLayoutCI.flags = 0;
			descSetLayoutCI.bindingCount = (uint32_t)bindings.size();
			descSetLayoutCI.pBindings = bindings.data();
			VK_ASSERT(vkCreateDescriptorSetLayout(mDevice->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayouts[id]), "Failed to create descriptor set layout");

			VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
			pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCI.pNext = nullptr;
			pipelineLayoutCI.flags = 0;
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayouts[id];
			VK_ASSERT(vkCreatePipelineLayout(mDevice->GetDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayouts[id]), "Failed to create descriptor set layout");

			const std::array<VkVertexInputAttributeDescription, 3> attributeDesc = Vertex::GetAttributeDescriptions();
			const std::array<VkVertexInputBindingDescription, 1> bindingDesc = Vertex::GetBindingDescription();

			VkPipelineVertexInputStateCreateInfo VISCI = {};
			VISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VISCI.pNext = nullptr;
			VISCI.flags = 0;
			VISCI.vertexAttributeDescriptionCount = (uint32_t)attributeDesc.size();
			VISCI.pVertexAttributeDescriptions = attributeDesc.data();
			VISCI.vertexBindingDescriptionCount = (uint32_t)bindingDesc.size();
			VISCI.pVertexBindingDescriptions = bindingDesc.data();

			VkPipelineInputAssemblyStateCreateInfo IASCI = {};
			IASCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			IASCI.pNext = nullptr;
			IASCI.flags = 0;
			IASCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			IASCI.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo VSCI = {};
			VSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			VSCI.pNext = nullptr;
			VSCI.flags = 0;
			VSCI.viewportCount = 1;
			VSCI.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo RSCI = {};
			RSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RSCI.pNext = nullptr;
			RSCI.flags = 0;
			RSCI.depthClampEnable = VK_FALSE;
			RSCI.rasterizerDiscardEnable = VK_FALSE;
			RSCI.polygonMode = VK_POLYGON_MODE_FILL;
			RSCI.lineWidth = 1.0f;
			RSCI.cullMode = VK_CULL_MODE_NONE;
			RSCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			RSCI.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo MSCI = {};
			MSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MSCI.pNext = nullptr;
			MSCI.flags = 0;
			MSCI.sampleShadingEnable = VK_FALSE;
			MSCI.rasterizationSamples = mCommander->GetPrimary()->msaa;

			VkPipelineDepthStencilStateCreateInfo DSSCI = {};
			DSSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			DSSCI.depthTestEnable = VK_TRUE;
			DSSCI.depthWriteEnable = VK_TRUE;
			DSSCI.depthCompareOp = VK_COMPARE_OP_LESS;
			DSSCI.depthBoundsTestEnable = VK_FALSE;
			DSSCI.stencilTestEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState CBAS = {};
			CBAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			CBAS.blendEnable = VK_FALSE;
			CBAS.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			CBAS.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			CBAS.colorBlendOp = VK_BLEND_OP_ADD;
			CBAS.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			CBAS.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			CBAS.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo CBSCI = {};
			CBSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			CBSCI.pNext = nullptr;
			CBSCI.flags = 0;
			CBSCI.logicOpEnable = VK_FALSE;
			CBSCI.logicOp = VK_LOGIC_OP_COPY;
			CBSCI.attachmentCount = 1;
			CBSCI.pAttachments = &CBAS;
			CBSCI.blendConstants[0] = 0.0f;
			CBSCI.blendConstants[1] = 0.0f;
			CBSCI.blendConstants[2] = 0.0f;
			CBSCI.blendConstants[3] = 0.0f;

			VkPipelineDynamicStateCreateInfo DSCI = {};
			DSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			DSCI.pNext = nullptr;
			DSCI.flags = 0;
			DSCI.dynamicStateCount = (uint32_t)dynamicStates.size();
			DSCI.pDynamicStates = dynamicStates.data();

			VkGraphicsPipelineCreateInfo pipelineCI = {};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.stageCount = 2;
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.pVertexInputState = &VISCI;
			pipelineCI.pInputAssemblyState = &IASCI;
			pipelineCI.pViewportState = &VSCI;
			pipelineCI.pRasterizationState = &RSCI;
			pipelineCI.pMultisampleState = &MSCI;
			pipelineCI.pDepthStencilState = &DSSCI;
			pipelineCI.pColorBlendState = &CBSCI;
			pipelineCI.pDynamicState = &DSCI;
			pipelineCI.layout = mPipelineLayouts[id];
			pipelineCI.renderPass = mCommander->GetPrimary()->renderPass;
			pipelineCI.subpass = 0;
			VK_ASSERT(vkCreateGraphicsPipelines(mDevice->GetDevice(), mPipelineCache, 1, &pipelineCI, nullptr, &mPipelines[id]), "Failed to create graphics pipeline");

			// destroy the shader module after usage
			vkDestroyShaderModule(mDevice->GetDevice(), vShader->Module(), nullptr);
			vkDestroyShaderModule(mDevice->GetDevice(), fShader->Module(), nullptr);
			
		}
	}
	*/
}