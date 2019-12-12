#include <snippets.h>
#include <VulkanBase.h>
#include <VulkanInitializer.hpp>
#include <Shader.hpp>
#include <Pipeline.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include <array>
#include "data.h"


class Triangle : public VulkanBase
{
	// Vertex buffer and attributes
	struct
	{
		VmaAllocation memory; // Handle to the device memory for this buffer
		VkBuffer buffer; // Handle to the Vulkan buffer object that the memory is bound to
	} vertices;

	// Index buffer
	struct
	{
		VmaAllocation memory;
		VkBuffer buffer;
		uint32_t count;
	} indices;

public:
	dhh::shader::Pipeline* trianglePipe;

	Triangle(): VulkanBase(true)
	{
		init();
		createTrianglePipeline();
		createVertexBuffer();
		createIndexBuffer();
		allocateCommandBuffers();
		buildCommandBuffers();
	}

	void allocateCommandBuffers()
	{
		commandBuffers.resize(3);

		VkCommandBufferAllocateInfo info = dhh::vk::initializer::commandBufferAllocateInfo(
			commandPool, 3, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		vkAllocateCommandBuffers(device, &info, commandBuffers.data());
	}

	void createVertexBuffer()
	{
		createBuffer(sizeof(Vertex) * vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, vertices.buffer,
		             vertices.memory);
		void* data;
		vmaMapMemory(allocator, vertices.memory, &data);
		memcpy(data, vertexData.data(), sizeof(vertexData) * vertexData.size());
		vmaUnmapMemory(allocator, vertices.memory);
	}

	void createIndexBuffer()
	{
		uint32_t index[] = {0, 1, 2};

		createBuffer(sizeof(Vertex), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, indices.buffer,
		             indices.memory);
		void* data;
		vmaMapMemory(allocator, indices.memory, &data);
		memcpy(data, index, sizeof(index));
		vmaUnmapMemory(allocator, indices.memory);
		indices.count = 3;
	}

	void createTrianglePipeline()
	{
		std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

		dhh::shader::Shader vertexShader(shaders_directory / "shader.vert");
		dhh::shader::Shader fragmentShader(shaders_directory / "shader.frag");

		trianglePipe = new dhh::shader::Pipeline(
			device, {&vertexShader, &fragmentShader},
			3, descriptorPool,
			renderPass,
			dhh::vk::initializer::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT),
			{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
			dhh::vk::initializer::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL,
			                                                           VK_CULL_MODE_NONE,
			                                                           VK_FRONT_FACE_COUNTER_CLOCKWISE),
			dhh::vk::initializer::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS),
			dhh::vk::initializer::pipelineViewportStateCreateInfo(1, 1),
			dhh::vk::initializer::pipelineColorBlendAttachmentState(
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT,
				false),
			dhh::vk::initializer::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		);
	}


	void buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.pNext = nullptr;

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		VkClearValue clearValues[2];
		clearValues[0].color = {{0.0f, 0.0f, 0.2f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0};

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = windowWidth;
		renderPassBeginInfo.renderArea.extent.height = windowHeight;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (int32_t i = 0; i < commandBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = swapchainFramebuffers[i];

			vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

			// Start the first sub pass specified in our default render pass setup by the base class
			// This will clear the color and depth attachment
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update dynamic viewport state
			VkViewport viewport = {};
			viewport.height = (float)windowHeight;
			viewport.width = (float)windowWidth;
			viewport.minDepth = (float)0.0f;
			viewport.maxDepth = (float)1.0f;
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor = {};
			scissor.extent.width = windowWidth;
			scissor.extent.height = windowHeight;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			// Bind descriptor sets describing shader binding points
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipe->pipelineLayout, 0, 1,
			                        &trianglePipe->descriptorSets[0][0], 0, nullptr);

			// Bind the rendering pipeline
			// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipe->pipeline);

			// Bind triangle vertex buffer (contains position and colors)
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertices.buffer, offsets);

			// Bind triangle index buffer
			vkCmdBindIndexBuffer(commandBuffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw indexed triangle
			vkCmdDrawIndexed(commandBuffers[i], indices.count, 1, 0, 0, 1);

			vkCmdEndRenderPass(commandBuffers[i]);

			// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

			vkEndCommandBuffer(commandBuffers[i]);
		}
	}
};


int main(int argc, char* argv[])
{
	try
	{
		Triangle app;


		while (glfwWindowShouldClose(app.window) != GLFW_TRUE)
		{
			glfwPollEvents();
			app.drawFrame();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
