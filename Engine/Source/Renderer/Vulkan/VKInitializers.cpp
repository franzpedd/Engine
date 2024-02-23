#include "epch.h"
#include "VKInitializers.h"

namespace Cosmos::vulkan
{
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t slot, VkDescriptorType type, VkShaderStageFlags stages, uint32_t count)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = slot;
		binding.descriptorType = type;
		binding.descriptorCount = count;
		binding.stageFlags = stages;
		binding.pImmutableSamplers = nullptr;

		return binding;
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.bindingCount = (uint32_t)bindings.size();
		ci.pBindings = bindings.data();

		return ci;
	}

	VkPipelineLayoutCreateInfo PipelineLayouCreateInfo(VkDescriptorSetLayout* layouts, uint32_t count)
	{
		VkPipelineLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.setLayoutCount = count;
		ci.pSetLayouts = layouts;

		return ci;
	}

	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes)
	{
		VkPipelineVertexInputStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.vertexBindingDescriptionCount = (uint32_t)bindings.size();
		ci.pVertexBindingDescriptions = bindings.data();
		ci.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
		ci.pVertexAttributeDescriptions = attributes.data();

		return ci;
	}

	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCrateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable)
	{
		VkPipelineInputAssemblyStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = flags;
		ci.topology = topology;
		ci.primitiveRestartEnable = primitiveRestartEnable;
		
		return ci;
	}

	VkPipelineTessellationStateCreateInfo PipelineTesselationStateCreateInfo(uint32_t patchControlPoints)
	{
		VkPipelineTessellationStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.patchControlPoints = patchControlPoints;
		
		return ci;
	}

	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags)
	{
		VkPipelineViewportStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = flags;
		ci.pViewports = nullptr; // using dynamic states
		ci.viewportCount = viewportCount;
		ci.pScissors = nullptr; // using dynamic states
		ci.scissorCount = scissorCount;
		
		return ci;
	}

	VkPipelineRasterizationStateCreateInfo PipelineRasterizationCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags)
	{
		VkPipelineRasterizationStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = flags;
		ci.polygonMode = polygonMode;
		ci.cullMode = cullMode;
		ci.frontFace = frontFace;
		ci.depthClampEnable = VK_FALSE;
		ci.rasterizerDiscardEnable = VK_FALSE;
		ci.lineWidth = 1.0f;

		return ci;
	}

	VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags)
	{
		VkPipelineMultisampleStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = flags;
		ci.rasterizationSamples = rasterizationSamples;
		ci.sampleShadingEnable = VK_FALSE;
		
		return ci;
	}

	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
	{
		VkPipelineDepthStencilStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.depthTestEnable = depthTestEnable;
		ci.depthWriteEnable = depthWriteEnable;
		ci.depthCompareOp = depthCompareOp;
		ci.back.compareOp = VK_COMPARE_OP_ALWAYS;

		return ci;
	}

	VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable)
	{
		VkPipelineColorBlendAttachmentState ci = {};
		ci.colorWriteMask = colorWriteMask;
		ci.blendEnable = blendEnable;

		ci.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ci.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ci.colorBlendOp = VK_BLEND_OP_ADD;
		ci.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ci.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ci.alphaBlendOp = VK_BLEND_OP_ADD;

		return ci;
	}

	VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments)
	{
		VkPipelineColorBlendStateCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.attachmentCount = attachmentCount;
		ci.pAttachments = pAttachments;

		ci.logicOpEnable = VK_FALSE;
		ci.logicOp = VK_LOGIC_OP_COPY;
		ci.blendConstants[0] = 0.0f;
		ci.blendConstants[1] = 0.0f;
		ci.blendConstants[2] = 0.0f;
		ci.blendConstants[3] = 0.0f;

		return ci;
	}

	VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& pDynamicStates, VkPipelineDynamicStateCreateFlags flags)
	{
		VkPipelineDynamicStateCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		ci.pDynamicStates = pDynamicStates.data();
		ci.dynamicStateCount = (uint32_t)pDynamicStates.size();
		ci.flags = flags;

		return ci;
	}
}