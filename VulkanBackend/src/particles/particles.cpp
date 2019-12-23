#include <snippets.h>
#include <VulkanBase.h>
#include <VulkanInitializer.hpp>
#include <Shader.hpp>
#include <Pipeline.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include <array>
#include <Camera.hpp>
#include "Input.hpp"
#include <random>

#define BODIES_COUNT 6144

struct Body
{
	glm::vec3 position;
	alignas(16) glm::vec3 last_position;
	alignas(16) glm::vec3 velocity;
	alignas(4) float mass;
};

dhh::camera::Camera camera;


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

	struct
	{
		VmaAllocation memory;
		VkBuffer buffer;
	} computeBuffer;


	struct
	{
		VmaAllocation memory;
		VkBuffer buffer;
	} cameraBuffer;

	struct Transforms
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
	};

public:
	dhh::shader::Pipeline* trianglePipe;
	dhh::shader::Pipeline* computePipe;
	dhh::shader::Pipeline* cachePipe;
	std::vector<Body> bodies;

	Triangle(): VulkanBase(false)
	{
		init();
		fillBodyInitialStates();
		createTrianglePipeline();
		createComputePipeline();
		createCameraBuffer();
		writeGraphicsDescriptorSet();
		createComputeBuffer();
		createVertexBuffer();
		buildCommandBuffers();
		writeComputeDescriptorSet();
		buildComputeCommandBuffers();
		compute();
	}

	void updateTransform()
	{
		dhh::input::processKeyboard(window, camera);

		void* data;
		vmaMapMemory(allocator, cameraBuffer.memory, &data);
		Transforms transforms{
			{glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / windowHeight, 0.1f, 1000.f)},
			{camera.GetViewMatrix()},
			{glm::mat4(1.f)}
		};

		memcpy(data, &transforms, sizeof(transforms));
		vmaUnmapMemory(allocator, cameraBuffer.memory);
	}

	void createCameraBuffer()
	{
		createBuffer(sizeof(glm::mat4) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
		             cameraBuffer.buffer, cameraBuffer.memory);
	}

	void writeGraphicsDescriptorSet()
	{
		VkDescriptorBufferInfo bufferInfo = dhh::vk::initializer::descriptorBufferInfo(
			cameraBuffer.buffer, 0, VK_WHOLE_SIZE);

		VkWriteDescriptorSet write = dhh::vk::initializer::writeDescriptorSet(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 0, trianglePipe->descriptorSets[0], &bufferInfo);

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	std::vector<glm::vec3> attractors = {
		glm::vec3(5.0f, 0.0f, 0.0f),
		glm::vec3(-5.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 5.0f),
		glm::vec3(0.0f, 0.0f, -5.0f),
		glm::vec3(0.0f, 4.0f, 0.0f),
		glm::vec3(0.0f, -8.0f, 0.0f),
	};

	const int PARTICLES_PER_ATTRACTOR = BODIES_COUNT / 6;

	void fillBodyInitialStates()
	{
		bodies.resize(BODIES_COUNT);
		std::default_random_engine rndEngine;
		std::normal_distribution<float> rndDist(0.0f, 1.0f);

		for (uint32_t i = 0; i < static_cast<uint32_t>(attractors.size()); i++)
		{
			for (uint32_t j = 0; j < PARTICLES_PER_ATTRACTOR; j++)
			{
				auto& body = bodies[i * PARTICLES_PER_ATTRACTOR + j];

				// First particle in group as heavy center of gravity
				if (j == 0)
				{
					body.last_position = body.position = glm::vec3(attractors[i] * 1.5f);
					body.velocity = glm::vec3(glm::vec3(0.0f));
					body.mass = 90000.0f;
				}
				else
				{
					// Position					
					glm::vec3 position(attractors[i] + glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine)) * 0.75f);
					float len = glm::length(glm::normalize(position - attractors[i]));
					position.y *= 2.0f - (len * len);

					// Velocity
					glm::vec3 angular = glm::vec3(0.5f, 1.5f, 0.5f) * (((i % 2) == 0) ? 1.0f : -1.0f);
					glm::vec3 velocity = glm::cross((position - attractors[i]), angular) + glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine) * 0.025f);

					float mass = (rndDist(rndEngine) * 0.5f + 0.5f) * 75.0f;
					body.position = body.last_position = glm::vec3(position);
					body.velocity = glm::vec4(velocity, 0.0f);
					body.mass = mass;
				}

				
			}
		}
	}

	void writeComputeDescriptorSet()
	{
		VkDescriptorBufferInfo bufferInfo = dhh::vk::initializer::descriptorBufferInfo(
			computeBuffer.buffer, 0, VK_WHOLE_SIZE);

		VkWriteDescriptorSet write = {};
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pBufferInfo = &bufferInfo;
		write.dstSet = computePipe->descriptorSets[0];
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
		write.dstSet = cachePipe->descriptorSets[0];
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	void compute()
	{
		VkFence completeFence;

		VkFenceCreateInfo fenceCreateInfo = dhh::vk::initializer::fenceCreateInfo();
		vkCreateFence(device, &fenceCreateInfo, nullptr, &completeFence);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &computeCmdBuf;

		auto now = std::chrono::high_resolution_clock::now();
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, completeFence);
		vkWaitForFences(device, 1, &completeFence, true, UINT64_MAX);
		vkResetFences(device, 1, &completeFence);

		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
		//	std::chrono::high_resolution_clock::now() - now).count() << std::endl;

		void* data;
		vmaMapMemory(allocator, computeBuffer.memory, &data);
		std::vector<Body> fuck(BODIES_COUNT);
		memcpy(fuck.data(), data, sizeof(Body) * BODIES_COUNT);
		vmaUnmapMemory(allocator, computeBuffer.memory);

		std::cout << glm::to_string(fuck[5000].position) << "\n";
	}

	VkCommandBuffer computeCmdBuf;

	void buildComputeCommandBuffers()
	{
		VkCommandBufferAllocateInfo info = dhh::vk::initializer::commandBufferAllocateInfo(
			commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		vkAllocateCommandBuffers(device, &info, &computeCmdBuf);
		VkCommandBufferBeginInfo beginInfo = dhh::vk::initializer::commandBufferBeginInfo(
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
		vkBeginCommandBuffer(computeCmdBuf, &beginInfo);

		// calculate

		vkCmdBindPipeline(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, computePipe->pipeline);
		vkCmdBindDescriptorSets(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, computePipe->pipelineLayout, 0, 1,
		                        computePipe->descriptorSets.data(), 0, nullptr);
		vkCmdDispatch(computeCmdBuf, BODIES_COUNT / 256, 1, 1);

		VkBufferMemoryBarrier barrier = {};
		barrier.buffer = computeBuffer.buffer;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.size = VK_WHOLE_SIZE;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(computeCmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		                     VK_NULL_HANDLE, 0, nullptr,
		                     1, &barrier, VK_NULL_HANDLE, nullptr);

		// cache old data
		vkCmdBindPipeline(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, cachePipe->pipeline);
		vkCmdBindDescriptorSets(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, cachePipe->pipelineLayout, 0, 1,
		                        cachePipe->descriptorSets.data(), 0, nullptr);
		vkCmdDispatch(computeCmdBuf, BODIES_COUNT / 256, 1, 1);

		vkEndCommandBuffer(computeCmdBuf);
	}


	void createComputeBuffer()
	{
		createBuffer(sizeof(Body) * bodies.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
		             computeBuffer.buffer, computeBuffer.memory);
		void* data;
		vmaMapMemory(allocator, computeBuffer.memory, &data);
		memcpy(data, bodies.data(), sizeof(Body) * bodies.size());
		vmaUnmapMemory(allocator, computeBuffer.memory);
	}

	void createVertexBuffer()
	{
		createBuffer(sizeof(glm::vec3) * 2 * BODIES_COUNT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
		             vertices.buffer, vertices.memory);
		positions.resize(BODIES_COUNT);
	}

	void createComputePipeline()
	{
		std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

		dhh::shader::Shader computeShader(shaders_directory / "nbody.comp");
		computePipe = new dhh::shader::Pipeline(device, {&computeShader}, descriptorPool);

		dhh::shader::Shader cacheShader(shaders_directory / "cache.comp");
		cachePipe = new dhh::shader::Pipeline(device, {&cacheShader}, descriptorPool);
	}

	void createTrianglePipeline()
	{
		std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

		dhh::shader::Shader vertexShader(shaders_directory / "shader.vert");
		dhh::shader::Shader fragmentShader(shaders_directory / "shader.frag");

		trianglePipe = new dhh::shader::Pipeline(
			device, {&vertexShader, &fragmentShader},
			descriptorPool,
			renderPass,
			dhh::vk::initializer::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT),
			{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
			dhh::vk::initializer::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_POINT,
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
			dhh::vk::initializer::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
		);
	}


	void buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = dhh::vk::initializer::commandBufferBeginInfo();

		// Set clear values for all framebuffer attachments with loadOp set to clear
		// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
		std::vector<VkClearValue> clearValues(2);
		clearValues[0].color = {{0.0f, 0.0f, 0.2f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0};

		for (int32_t i = 0; i < commandBuffers.size(); ++i)
		{
			VkRenderPassBeginInfo renderPassBeginInfo = dhh::vk::initializer::renderPassBeginInfo(
				clearValues,
				framebuffers[i],
				renderPass,
				windowWidth,
				windowHeight);

			vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

			// Start the first sub pass specified in our default render pass setup by the base class
			// This will clear the color and depth attachment
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update dynamic viewport state
			VkViewport viewport = {};
			viewport.height = -static_cast<float>(windowHeight);
			viewport.width = static_cast<float>(windowWidth);
			viewport.minDepth = static_cast<float>(0.0f);
			viewport.maxDepth = static_cast<float>(1.0f);
			viewport.x = 0;
			viewport.y = windowHeight;
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor = {};
			scissor.extent.width = windowWidth;
			scissor.extent.height = windowHeight;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			// Bind descriptor sets describing shader binding points
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipe->pipelineLayout, 0,
			                        1,
			                        &trianglePipe->descriptorSets[0], 0, nullptr);

			// Bind the rendering pipeline
			// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipe->pipeline);

			// Bind triangle vertex buffer (contains position and colors)
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertices.buffer, offsets);

			vkCmdDraw(commandBuffers[i], BODIES_COUNT, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

			vkEndCommandBuffer(commandBuffers[i]);
		}
	}

	bool colorSet = false;

	struct vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

	std::vector<vertex> positions;

	void updateVertexBuffer()
	{
		void* data;
		vmaMapMemory(allocator, computeBuffer.memory, &data);
		std::vector<Body> fuck(BODIES_COUNT);
		memcpy(fuck.data(), data, sizeof(Body) * BODIES_COUNT);
		vmaUnmapMemory(allocator, computeBuffer.memory);
		std::default_random_engine rndEngine;
		std::normal_distribution<float> rndDist(0.0f, 1.0f);


		for (int i = 0; i < BODIES_COUNT; ++i)
		{
			positions[i].pos = fuck[i].position;
			if (!colorSet)
				positions[i].color = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
		}
		std::cout << glm::to_string(positions[6000].pos) << "\n";
		colorSet = true;

		vmaMapMemory(allocator, vertices.memory, &data);
		memcpy(data, positions.data(), sizeof(vertex) * BODIES_COUNT);
		vmaUnmapMemory(allocator, vertices.memory);
	}
};


int main(int argc, char* argv[])
{
	try
	{
		Triangle app;

		int anchor = 0;
		double years = 0;
		while (glfwWindowShouldClose(app.window) != GLFW_TRUE)
		{
			app.updateTransform();
			app.updateVertexBuffer();
			app.drawFrame();
			app.compute();
			glfwPollEvents();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
