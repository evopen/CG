#include "VulkanBase.h"
#include "VulkanTool.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include "data.h"
#include <iostream>
#include "VulkanType.h"

const int VertexBindingCount = 1;
const int VertexAttrCount = 2;


class Triangle : public VulkanBase
{
public:
	Triangle(bool enableValidation)
		: VulkanBase(enableValidation)
	{
	}

	void prepare()
	{
		loadModel();
		createBuffers();
		createDescriptorPool();
		createDescriptorSetLayout();
		createDescriptorSets();
		createGraphicsPipeline();
	}

private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkPipeline modelPipeline;
	VkBuffer vertexBuffer;
	VmaAllocation vertexAllocation;
	VkBuffer modelVertexBuffer;
	VmaAllocation modelVertexAllocation;
	VkBuffer modelIndexBuffer;
	VmaAllocation modelIndexAllocation;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<float> vertices;
	std::vector<uint32_t> indices;

	void loadModel()
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("models/treasure_smooth.dae",
		                                         aiProcess_FlipWindingOrder | aiProcess_Triangulate |
		                                         aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace |
		                                         aiProcess_GenSmoothNormals);
		std::cout << scene->mNumMeshes << "\n";

		for (uint32_t i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			for (uint32_t j = 0; j < mesh->mNumVertices; j++)
			{
				const aiVector3D* pos = &(mesh->mVertices[j]);
				vertices.push_back(pos->x);
				vertices.push_back(pos->y);
				vertices.push_back(pos->z);
			}
			uint32_t indexBase = static_cast<uint32_t>(indices.size());
			for (uint32_t j = 0; j < mesh->mNumFaces; j++)
			{
				const aiFace& face = mesh->mFaces[j];
				if (face.mNumIndices != 3)
					continue;
				indices.push_back(indexBase + face.mIndices[0]);
				indices.push_back(indexBase + face.mIndices[1]);
				indices.push_back(indexBase + face.mIndices[2]);
			}
		}
		// std::cout << vertices.size() << "\n" << vertices[8000];
		std::cout << indices.size() << "\n";
	}

	void createDescriptorPool()
	{
		VkDescriptorPoolSize uboPoolSize = {};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = 3;
		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.maxSets = 3;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &uboPoolSize;
		vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool);
	}

	void createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(3, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = 3;
		allocateInfo.pSetLayouts = layouts.data();
		descriptorSets.resize(3);
		vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data());

		for (size_t i = 0; i < descriptorSets.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(engine::type::UniformBufferObject);
			VkWriteDescriptorSet writes = {};
			writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes.descriptorCount = 1;
			writes.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writes.dstBinding = 0;
			writes.dstSet = descriptorSets[i];
			writes.pBufferInfo = &bufferInfo;
			vkUpdateDescriptorSets(device, 1, &writes, 0, nullptr);
		}
	}

	void createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboBinding = {};
		uboBinding.binding = 0;
		uboBinding.descriptorCount = 1;
		uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {};
		setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutCreateInfo.bindingCount = 1;
		setLayoutCreateInfo.pBindings = &uboBinding;
		vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout);
	}

	void createGraphicsPipeline()
	{
		VkShaderModule vertShaderModule = engine::tool::createShaderModule(device, "shaders/vert.spv");
		VkShaderModule fragShaderModule = engine::tool::createShaderModule(device, "shaders/frag.spv");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		std::array<VkVertexInputBindingDescription, VertexBindingCount> vertexInputBindingDescriptions;
		vertexInputBindingDescriptions[0].binding = 0;
		vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindingDescriptions[0].stride = sizeof(Vertex);

		std::array<VkVertexInputAttributeDescription, VertexAttrCount> vertexInputAttributeDescriptions;
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()
		);
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent.width = width;
		scissor.extent.height = height;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multiSampleInfo = {};
		multiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampleInfo.sampleShadingEnable = VK_FALSE;
		multiSampleInfo.rasterizationSamples = sampleCount;

		VkPipelineDepthStencilStateCreateInfo depthInfo = {};
		depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthInfo.depthTestEnable = VK_TRUE;
		depthInfo.depthWriteEnable = VK_TRUE;
		depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthInfo.depthBoundsTestEnable = VK_FALSE;
		depthInfo.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.flags = VK_NULL_HANDLE;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pTessellationState = nullptr;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &rasterizer;
		pipelineCreateInfo.pMultisampleState = &multiSampleInfo;
		pipelineCreateInfo.pDepthStencilState = &depthInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlending;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.layout = pipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = 0;

		vertexInputBindingDescriptions[0].binding = 0;
		vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindingDescriptions[0].stride = sizeof(float) * 3;

		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].offset = 0;

		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
		vertexInputState.vertexAttributeDescriptionCount = 1;
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

		vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &modelPipeline);
	}

	void createBuffers()
	{
		createVertexBuffers();
		createUniformBuffer();
	}

	void createUniformBuffer()
	{
		VkDeviceSize size = sizeof(engine::type::UniformBufferObject);
		uniformBuffers.resize(3);
		uniformBufferAllocation.resize(3);
		for (int i = 0; i < 3; i++)
		{
			engine::tool::createBuffer(allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
			                           uniformBuffers[i], uniformBufferAllocation[i]);
		}
	}

	void createVertexBuffers()
	{
		// VkDeviceSize size = sizeof(Vertex) * vertex.size();
		// engine::tool::createVertexInputBuffer(allocator, size, false, vertexBuffer, vertexAllocation, vertex.data());
		VkDeviceSize modelSize = vertices.size() * sizeof(float);
		VkDeviceSize indexSize = indices.size() * sizeof(uint32_t);
		engine::tool::createVertexInputBuffer(allocator, modelSize, false, modelVertexBuffer, modelVertexAllocation,
		                                      vertices.data());
		engine::tool::createIndexInputBuffer(allocator, indexSize, false, modelIndexBuffer, modelIndexAllocation,
		                                     indices.data());
	}

	void updateCommandBuffer(uint32_t i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		std::vector<VkClearValue> clearValues(2);
		clearValues[0].color = {0, 0, 1};
		clearValues[1].depthStencil = {1, 0};

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.framebuffer = swapchainFramebuffers[i];
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// show time
		VkDeviceSize offsets = 0;

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		                        &descriptorSets[i], 0, nullptr);
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &modelVertexBuffer, &offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], modelIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);

		// vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		// vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		//                         &descriptorSets[i], 0, nullptr);
		// vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offsets);
		// vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		///

		vkCmdEndRenderPass(commandBuffers[i]);
		vkEndCommandBuffer(commandBuffers[i]);
	}

	void updateUniformBuffer(uint32_t i)
	{
		engine::type::UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.f);
		ubo.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		ubo.proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
		void* data;
		vmaMapMemory(allocator, uniformBufferAllocation[i], &data);
		memcpy(data, &ubo, sizeof(ubo));
		vmaUnmapMemory(allocator, uniformBufferAllocation[i]);
	}


	void updateData(uint32_t imageIndex) override
	{
		updateUniformBuffer(imageIndex);
		updateCommandBuffer(imageIndex);
	}
};

int main(int argc, char* argv[])
{
	Triangle app(true);
	app.prepare();

	app.loop();
	return 0;
}
