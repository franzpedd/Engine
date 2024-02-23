#pragma once

#include <vulkan/vulkan.h>

namespace Cosmos::vulkan
{
	// descriptors

	// returns a semi-fullfilled VkDescriptorSetLayoutBinding structure
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t slot, VkDescriptorType type, VkShaderStageFlags stages, uint32_t count = 1);

	// returns a semi-fullfilled VkDescriptorSetLayoutCreateInfo structure
	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings);


	// pipeline

	// returns a semi-fullfilled VkPipelineLayoutCreateInfo structure
	VkPipelineLayoutCreateInfo PipelineLayouCreateInfo(VkDescriptorSetLayout* layouts, uint32_t count = 1);

	// returns a semi-fullfilled VkPipelineVertexInputStateCreateInfo structure
	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes);

	// returns a semi-fullfilled VkPipelineInputAssemblyStateCreateInfo structure
	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCrateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable);

	// returns a semi-fullfilled VkPipelineTessellationStateCreateInfo structure
	VkPipelineTessellationStateCreateInfo PipelineTesselationStateCreateInfo(uint32_t patchControlPoints);

	// returns a semi-fullfilled VkPipelineViewportStateCreateInfo structure
	VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags = 0);

	// returns a semi-fullfilled VkPipelineRasterizationStateCreateInfo structure
	VkPipelineRasterizationStateCreateInfo PipelineRasterizationCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags = 0);

	// returns a semi-fullfilled VkPipelineMultisampleStateCreateInfo structure
	VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags = 0);

	// returns a semi-fullfilled VkPipelineDepthStencilStateCreateInfo structure
	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp);

	// returns a semi-fullfilled VkPipelineColorBlendAttachmentState structure
	VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable);

	// returns a semi-fullfilled VkPipelineColorBlendStateCreateInfo structure
	VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments);
	
	// returns a semi-fullfilled VkPipelineDynamicStateCreateInfo structure
	VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& pDynamicStates, VkPipelineDynamicStateCreateFlags flags = 0);
}