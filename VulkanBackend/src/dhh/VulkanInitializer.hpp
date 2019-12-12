#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace dhh::vk::initializer
{
	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode,
	                                                                                   VkCullModeFlags cullMode,
	                                                                                   VkFrontFace frontFace)
	{
		VkPipelineRasterizationStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.depthClampEnable = VK_FALSE;
		info.rasterizerDiscardEnable = VK_FALSE;
		info.polygonMode = polygonMode;
		info.lineWidth = 1.0f;
		info.cullMode = cullMode;
		info.frontFace = frontFace;
		info.depthBiasEnable = VK_FALSE;
		return info;
	}

	inline VkCommandBufferBeginInfo commandBufferBeginInfo()
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		return info;
	}

	inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count,
	                                                             VkCommandBufferLevel level)
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = level;
		info.commandPool = pool;
		info.commandBufferCount = count;
		return info;
	}


	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
	{
		VkPipelineDepthStencilStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.depthTestEnable = depthTestEnable;
		info.depthWriteEnable = depthWriteEnable;
		info.depthCompareOp = depthCompareOp;
		return info;
	}

	inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits samples)
	{
		VkPipelineMultisampleStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.rasterizationSamples = samples;
		return info;
	}

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
		const std::vector<VkDynamicState>& dynamicStates)
	{
		VkPipelineDynamicStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.dynamicStateCount = dynamicStates.size();
		info.pDynamicStates = dynamicStates.data();
		return info;
	}

	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
		VkColorComponentFlags colorWriteMask,
		VkBool32 blendEnable)
	{
		VkPipelineColorBlendAttachmentState info = {};
		info.colorWriteMask = colorWriteMask;
		info.blendEnable = blendEnable;
		return info;
	}

	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
		VkPrimitiveTopology topology)
	{
		VkPipelineInputAssemblyStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.topology = topology;
		return info;
	}

	inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
		uint32_t scissorCount, uint32_t viewportCount)
	{
		VkPipelineViewportStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		info.scissorCount = scissorCount;
		info.viewportCount = viewportCount;
		return info;
	}

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
		const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		VkPipelineVertexInputStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.vertexBindingDescriptionCount = bindingDescriptions.size();
		info.pVertexBindingDescriptions = bindingDescriptions.data();
		info.vertexAttributeDescriptionCount = attributeDescriptions.size();
		info.pVertexAttributeDescriptions = attributeDescriptions.data();
		return info;
	}

	inline VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
		const std::vector<VkPipelineShaderStageCreateInfo>& stages,
		VkRenderPass renderPass, VkPipelineLayout layout)
	{
		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.stageCount = stages.size();
		info.pStages = stages.data();
		info.renderPass = renderPass;
		info.layout = layout;
		return info;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
		std::vector<VkPipelineColorBlendAttachmentState>& attachments)
	{
		VkPipelineColorBlendStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.attachmentCount = attachments.size();
		info.pAttachments = attachments.data();
		return info;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
		VkPipelineColorBlendAttachmentState& attachments, uint32_t attachmentCount = 1)
	{
		VkPipelineColorBlendStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.attachmentCount = attachmentCount;
		info.pAttachments = &attachments;
		return info;
	}

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(VkDescriptorSetLayout& setLayout,
	                                                           uint32_t setLayoutCount = 1)
	{
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = setLayoutCount;
		info.pSetLayouts = &setLayout;
		return info;
	}
}
