#pragma once

#include "Util/Math.h"

#include <vulkan/vulkan.h>
#include <array>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class VKDevice;
	class VKShader;

	class VKDrawable
	{
	public:

		// constructor
		VKDrawable(std::shared_ptr<VKDevice>& device);

		// destructor
		virtual ~VKDrawable();

		// creates a graphics pipeline
		virtual void CreateGraphicsPipeline(VkRenderPass& renderPass, VkPipelineCache& cache) = 0;

		// creates drawable descriptor set layout
		virtual void CreateDescriptorSetLayout() = 0;

		// creates drawable descriptor set
		virtual void CreateDescriptorSets() = 0;

		// creates drawable descriptor pool / TEMPORARY
		virtual void CreateDescriptorPool() = 0;

	protected:

		std::shared_ptr<VKDevice>& mDevice;

	};

	class UBO : public VKDrawable
	{
	public:

		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;

			// returns UBO binding description
			static VkVertexInputBindingDescription BindingDescription();

			// returns UBO attribute descriptions
			static std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions();
		};

	public:

		// returns a smart pointer to a new UBO
		static std::shared_ptr<UBO> Create(std::shared_ptr<VKDevice>& device, VkRenderPass& renderPass, VkPipelineCache& pipelineCache);

		// constructor
		UBO(std::shared_ptr<VKDevice>& device, VkRenderPass& renderPass, VkPipelineCache& pipelineCache);

		// destructor
		virtual ~UBO();

	public:

		// creates a graphics pipeline
		virtual void CreateGraphicsPipeline(VkRenderPass& renderPass, VkPipelineCache& pipelineCache) override;

		// creates drawable descriptor set layout
		virtual void CreateDescriptorSetLayout() override;

		// creates drawable descriptor set
		virtual void CreateDescriptorSets() override;

		// creates drawable descriptor pool / TEMPORARY
		virtual void CreateDescriptorPool() override;

	public:

		// creates the uniform buffer object
		void CreateUniformBuffers();

	private:

		VkRenderPass& mRenderPass;
		VkPipelineCache& mPipelineCache;

		std::shared_ptr<VKShader> mVertex;
		std::shared_ptr<VKShader> mFragment;
		std::vector<VkBuffer> mBuffers;
		std::vector<VkDeviceMemory> mBuffersMemory;
		std::vector<void*> mBuffersMapped;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = {};
	};
}