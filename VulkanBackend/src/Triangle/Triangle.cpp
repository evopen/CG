#include "data.h"

#include <Input.hpp>
#include <Pipeline.hpp>
#include <Shader.hpp>
#include <VulkanBase.h>
#include <VulkanInitializer.hpp>
#include <glm/gtx/string_cast.hpp>
#include <snippets.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <vector>


class Triangle : public VulkanBase
{
    struct Transforms
    {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;
    };

    // Vertex buffer and attributes
    struct
    {
        VmaAllocation memory;  // Handle to the device memory for this buffer
        VkBuffer buffer;  // Handle to the Vulkan buffer object that the memory is bound to
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

public:
    dhh::shader::Pipeline* trianglePipe;
    dhh::shader::Pipeline* computePipe;

    dhh::camera::Camera camera;


    Triangle() : VulkanBase(true)
    {
        dhh::input::camera = &camera;
        init();
        createTrianglePipeline();
        CreateComputePipeline();
        CreateVertexBuffer();
        createIndexBuffer();
        createComputeBuffer();
        CreateCameraBuffer();
        WriteGraphicsDescriptorSet();
        buildCommandBuffers();
        writeComputeDescriptorSet();
        BuildComputeCommandBuffers();
        Compute();
    }

    void WriteGraphicsDescriptorSet()
    {
        VkDescriptorBufferInfo bufferInfo =
            dhh::vk::initializer::descriptorBufferInfo(cameraBuffer.buffer, 0, VK_WHOLE_SIZE);

        VkWriteDescriptorSet write = dhh::vk::initializer::writeDescriptorSet(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 0, trianglePipe->descriptorSets[0], &bufferInfo);

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    void CreateCameraBuffer()
    {
        createBuffer(sizeof(glm::mat4) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
            cameraBuffer.buffer, cameraBuffer.memory);
    }

    void writeComputeDescriptorSet()
    {
        VkDescriptorBufferInfo bufferInfo =
            dhh::vk::initializer::descriptorBufferInfo(computeBuffer.buffer, 0, VK_WHOLE_SIZE);

        VkWriteDescriptorSet write = {};
        write.descriptorCount      = 1;
        write.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pBufferInfo          = &bufferInfo;
        write.dstSet               = computePipe->descriptorSets[0];

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    void Compute()
    {
        VkFence completeFence;
        VkFenceCreateInfo fenceCreateInfo = dhh::vk::initializer::fenceCreateInfo();
        vkCreateFence(device, &fenceCreateInfo, nullptr, &completeFence);

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &computeCmdBuf;

        auto now = std::chrono::high_resolution_clock::now();
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, completeFence);
        vkWaitForFences(device, 1, &completeFence, true, UINT64_MAX);

        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - now)
                         .count()
                  << std::endl;

        void* data;
        vmaMapMemory(allocator, computeBuffer.memory, &data);
        std::vector<glm::vec4> fuck(1024);
        memcpy(fuck.data(), data, sizeof(glm::vec4) * 1024);
        vmaUnmapMemory(allocator, computeBuffer.memory);

        for (int i = 0; i < 10; ++i)
        {
            std::cout << glm::to_string(fuck[i]) << "\n";
        }
    }

    VkCommandBuffer computeCmdBuf;

    void BuildComputeCommandBuffers()
    {
        VkCommandBufferAllocateInfo info =
            dhh::vk::initializer::commandBufferAllocateInfo(commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        vkAllocateCommandBuffers(device, &info, &computeCmdBuf);
        VkCommandBufferBeginInfo beginInfo = dhh::vk::initializer::commandBufferBeginInfo();
        vkBeginCommandBuffer(computeCmdBuf, &beginInfo);
        vkCmdBindPipeline(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, computePipe->pipeline);
        vkCmdBindDescriptorSets(computeCmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, computePipe->pipelineLayout, 0, 1,
            computePipe->descriptorSets.data(), 0, nullptr);
        vkCmdDispatch(computeCmdBuf, 16, 1, 1);
        vkEndCommandBuffer(computeCmdBuf);
    }


    void createComputeBuffer()
    {
        createBuffer(sizeof(glm::vec4) * 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
            computeBuffer.buffer, computeBuffer.memory);
        void* data;
        std::vector<glm::vec4> fuck(1024);
        for (auto& point : fuck)
        {
            point = glm::vec4(0);
        }
        vmaMapMemory(allocator, computeBuffer.memory, &data);
        memcpy(data, fuck.data(), sizeof(glm::vec4) * 1024);
        vmaUnmapMemory(allocator, computeBuffer.memory);
    }

    void CreateVertexBuffer()
    {
        createBuffer(sizeof(Vertex) * vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
            vertices.buffer, vertices.memory);
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

    void CreateComputePipeline()
    {
        std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

        dhh::shader::Shader computeShader(shaders_directory / "shader.comp");
        computePipe = new dhh::shader::Pipeline(device, {&computeShader}, descriptorPool);
    }

    void createTrianglePipeline()
    {
        std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

        dhh::shader::Shader vertexShader(shaders_directory / "shader.vert");
        dhh::shader::Shader fragmentShader(shaders_directory / "shader.frag");

        trianglePipe = new dhh::shader::Pipeline(device, {&vertexShader, &fragmentShader}, descriptorPool, renderPass,
            dhh::vk::initializer::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT),
            {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
            dhh::vk::initializer::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE),
            dhh::vk::initializer::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS),
            dhh::vk::initializer::pipelineViewportStateCreateInfo(1, 1),
            dhh::vk::initializer::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                                                        | VK_COLOR_COMPONENT_B_BIT
                                                                        | VK_COLOR_COMPONENT_A_BIT,
                false),
            dhh::vk::initializer::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST));
    }


    void buildCommandBuffers()
    {
        VkCommandBufferBeginInfo cmdBufInfo = dhh::vk::initializer::commandBufferBeginInfo();

        // Set clear values for all framebuffer attachments with loadOp set to clear
        // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to
        // set clear values for both
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color        = {{0.0f, 0.0f, 0.2f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        for (int32_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkRenderPassBeginInfo renderPassBeginInfo = dhh::vk::initializer::renderPassBeginInfo(
                clearValues, framebuffers[i], renderPass, windowWidth, windowHeight);

            vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

            // Start the first sub pass specified in our default render pass setup by the base class
            // This will clear the color and depth attachment
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Update dynamic viewport state
            VkViewport viewport = {};
            viewport.height     = -static_cast<float>(windowHeight);
            viewport.width      = static_cast<float>(windowWidth);
            viewport.minDepth   = static_cast<float>(0.0f);
            viewport.maxDepth   = static_cast<float>(1.0f);
            viewport.x          = 0;
            viewport.y          = windowHeight;
            vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

            // Update dynamic scissor state
            VkRect2D scissor      = {};
            scissor.extent.width  = windowWidth;
            scissor.extent.height = windowHeight;
            scissor.offset.x      = 0;
            scissor.offset.y      = 0;
            vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

            // Bind descriptor sets describing shader binding points
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipe->pipelineLayout, 0,
                1, &trianglePipe->descriptorSets[0], 0, nullptr);

            // Bind the rendering pipeline
            // The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the
            // states specified at pipeline creation time
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

    void updateTransform()
    {
        dhh::input::processKeyboard(window, camera);

        void* data;
        vmaMapMemory(allocator, cameraBuffer.memory, &data);
        Transforms transforms{
            {glm::perspective(glm::radians(camera.Zoom), (float) windowWidth / windowHeight, 0.1f, 1000.f)},
            {camera.GetViewMatrix()}, {glm::mat4(1.f)}};

        memcpy(data, &transforms, sizeof(transforms));
        vmaUnmapMemory(allocator, cameraBuffer.memory);
    }
};


int main(int argc, char* argv[])
{
    try
    {
        Triangle app;


        while (!glfwWindowShouldClose(app.window))
        {
            app.updateTransform();
            app.drawFrame();
            glfwPollEvents();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
