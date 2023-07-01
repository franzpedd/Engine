#pragma once

#include "Core/Defines.h"

#include "VKBuffer.h"
#include "VKDevice.h"
#include "VKInstance.h"
#include "VKSwapchain.h"
#include "VKTypes.h"

namespace Cosmos
{
	class COSMOS_API VKRenderer
	{
	private:

		struct Settings
		{
			bool Multisampling = true;
			VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_4_BIT;
		};

		struct Specification
		{
			bool Validations = true;
			const char* AppName = nullptr;
			const char* EngineName = nullptr;
			VkCommandPool CommandPool = VK_NULL_HANDLE;
			VkRenderPass RenderPass = VK_NULL_HANDLE;
			VkPipelineCache PipelineCache = VK_NULL_HANDLE;
			VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
			MultiSampleTarget MultiSampleTarget = {};
			DepthStencil DepthStencil = {};
			Syncronization Syncronization = {};
			std::vector<VkFramebuffer> FrameBuffers;
			std::vector<VkCommandBuffer> CommandBuffers;
		};

	public:

		// returns a smart-ptr to a new vkscene
		static std::shared_ptr<VKRenderer> Create(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations = true);

		// constructor
		VKRenderer(std::shared_ptr<Platform>& platform, const char* appName, const char* engineName, bool validations = true);

		// destructor
		~VKRenderer();

		// returns renderer's instance
		inline std::shared_ptr<VKInstance>& GetInstance() { return m_Instance; }

		// returns renderer's device
		inline std::shared_ptr<VKDevice>& GetDevice() { return m_Device; }

		// returns renderer's swapchain
		inline std::shared_ptr<VKSwapchain>& GetSwapchain() { return m_Swapchain; }

		// returns renderer's specification
		inline Specification& GetSpecification() { return m_Specification; }

		// returns renderer's settings
		inline Settings& GetSettings() { return m_Settings; }

	public:

		// initializes pre-configuration of the renderer
		void Prepare();

		// renders the current frame
		void Render();

	private:

		// creates the main commandpool
		void CreateCommandPool();

		// creates used render passed
		void CreateRenderPass();

		// creates the syncronization system between cpu and gpu
		void CreateSyncSystem();

		// creates the command buffers
		void CreateCommandBuffers();

		// auto-configures the framebuffer
		void SetupFramebuffer();

	private:

		std::shared_ptr<Platform>& m_Platform;
		std::shared_ptr<VKInstance> m_Instance;
		std::shared_ptr<VKDevice> m_Device;
		std::shared_ptr<VKSwapchain> m_Swapchain;
		Specification m_Specification;
		Settings m_Settings;

		// this will move to a VKScene in the future, right now I just want for it to work
		std::vector<UniformBufferSets> m_UniformBuffers;
		std::vector<DescriptorSets> m_DescriptorSets;
	};
}