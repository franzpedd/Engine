#pragma once

#include "Defines.h"
#include "Util/Math.h"
#include <vulkan/vulkan.h>
#include <map>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class VKDevice;
	class VKShader;

	class VKGraphicsPipeline
	{
	public:

		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		/* herlper to pre-configure the graphics pipeline,
			* destruction is handled by the class, but creation must be done outside class-scope
		*/
		struct InitializerList
		{
			VkRenderPass& renderPass;
			VkPipelineCache& pipelineCache;

			std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			std::vector<VkVertexInputBindingDescription> bindings;
			std::vector<VkVertexInputAttributeDescription> attributes;

			std::shared_ptr<VKShader> vertexShader;
			std::shared_ptr<VKShader> fragmentShader;

			// customize before creation
			VkCullModeFlags cullMode = VK_CULL_MODE_NONE;

			// constructor
			InitializerList(VkRenderPass& renderPass, VkPipelineCache& pipelineCache)
				: renderPass(renderPass), pipelineCache(pipelineCache) { }
			
			// destructor
			~InitializerList() = default;
		};

	public:

		// returns a smart-ptr to a new graphics pipeline
		static std::shared_ptr<VKGraphicsPipeline> Create(std::shared_ptr<VKDevice>& device, InitializerList& initializerList);

		// constructor
		VKGraphicsPipeline(std::shared_ptr<VKDevice>& device, InitializerList& initializerList);

		// destructor
		~VKGraphicsPipeline() = default;

		// returns the pipeline object
		inline VkPipeline& GetPipeline() { return mPipeline; }

		// returns the pipeline layout object
		inline VkPipelineLayout& GetPipelineLayout() { return mPipelineLayout; }

		// returns a reference to the descriptor set vector
		inline std::vector<VkDescriptorSet>& GetDescriptorSets() { return mDescriptorSets; }

		// returns a reference to the ubo's
		inline std::vector<void*>& AccessUniformBuffers() {	return mUniformBuffersMapped; }

	public:

		// cleans used resources (not on destructor as it may be called after device is already destroyed)
		void Destroy();

	private:

		// creates the pipeline object
		void CreatePipeline();

		// creates the ubo
		void CreateUniformBufferObject();

		// creates the descriptor pool and descriptor set
		void CreateDescriptors();

	private:

		std::shared_ptr<VKDevice>& mDevice;

		InitializerList mInitializerList;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipeline mPipeline = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;
	};

	class VKPipelineLibrary
	{
	public:

		// constructor
		VKPipelineLibrary() = default;

		// destructor
		~VKPipelineLibrary() = default;

		// allows to edit private graphics table outside library
		inline std::map<const char*, std::shared_ptr<VKGraphicsPipeline>>& AccessGraphicsTable() { return mGraphicsTable; }

	public:

		// cleans all libraries
		void DestroyAllPipelines();

	private:

		std::map<const char*, std::shared_ptr<VKGraphicsPipeline>> mGraphicsTable;
	}; 
}