#include "Camera.hpp"
#include "Input.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"
#include "VulkanBase.h"
#include "VulkanInitializer.hpp"

#include <glm/gtx/string_cast.hpp>

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <random>
#include <vector>

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
        VmaAllocation memory;  // Handle to the device memory for this buffer
        VkBuffer buffer;  // Handle to the Vulkan buffer object that the memory is bound to
    } vertices_;

    // Index buffer
    struct
    {
        VmaAllocation memory;
        VkBuffer buffer;
        uint32_t count;
    } indices_;

    struct
    {
        VmaAllocation memory;
        VkBuffer buffer;
    } computeBuffer_;


    struct
    {
        VmaAllocation memory;
        VkBuffer buffer;
    } cameraBuffer_;

    struct Transforms
    {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;
    };

public:
    dhh::shader::Pipeline* triangle_pipe;
    dhh::shader::Pipeline* comput_pipe;
    dhh::shader::Pipeline* cach_pipe;
    std::vector<Body> bodies;

    Triangle() : VulkanBase(false)
    {
        init();
        FillBodyInitialStates();
        CreateTrianglePipeline();
        CreateComputePipeline();
        CreateCameraBuffer();
        WriteGraphicsDescriptorSet();
        CreateComputeBuffer();
        CreateVertexBuffer();
        BuildCommandBuffers();
        WriteComputeDescriptorSet();
        BuildComputeCommandBuffers();
        Compute();
    }

    void UpdateTransform()
    {
        dhh::input::processKeyboard(window, camera);

        void* data;
        vmaMapMemory(allocator, cameraBuffer_.memory, &data);
        Transforms transforms{
            {glm::perspective(glm::radians(camera.Zoom), (float) windowWidth / windowHeight, 0.1F, 1000.F)},
            {camera.GetViewMatrix()}, {glm::mat4(1.F)}};

        memcpy(data, &transforms, sizeof(transforms));
        vmaUnmapMemory(allocator, cameraBuffer_.memory);
    }

    void CreateCameraBuffer()
    {
        createBuffer(sizeof(glm::mat4) * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
            cameraBuffer_.buffer, cameraBuffer_.memory);
    }

    void WriteGraphicsDescriptorSet()
    {
        VkDescriptorBufferInfo buffer_info =
            dhh::vk::initializer::descriptorBufferInfo(cameraBuffer_.buffer, 0, VK_WHOLE_SIZE);

        VkWriteDescriptorSet write = dhh::vk::initializer::writeDescriptorSet(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 0, triangle_pipe->descriptorSets[0], &buffer_info);

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    std::vector<glm::vec3> attractors = {
        glm::vec3(5.0F, 0.0F, 0.0F),
        glm::vec3(-5.0F, 0.0F, 0.0F),
        glm::vec3(0.0F, 0.0F, 5.0F),
        glm::vec3(0.0F, 0.0F, -5.0F),
        glm::vec3(0.0F, 4.0F, 0.0F),
        glm::vec3(0.0F, -8.0F, 0.0F),
    };

    const int kParticlesPerAttractor = BODIES_COUNT / 6;

    void FillBodyInitialStates()
    {
        bodies.resize(BODIES_COUNT);
        std::default_random_engine rnd_engine;
        std::normal_distribution<float> rnd_dist(0.0F, 1.0F);

        for (uint32_t i = 0; i < static_cast<uint32_t>(attractors.size()); i++)
        {
            for (uint32_t j = 0; j < kParticlesPerAttractor; j++)
            {
                auto& body = bodies[i * kParticlesPerAttractor + j];

                // First particle in group as heavy center of gravity
                if (j == 0)
                {
                    body.last_position = body.position = glm::vec3(attractors[i] * 1.5F);
                    body.velocity                      = glm::vec3(glm::vec3(0.0F));
                    body.mass                          = 90000.0F;
                }
                else
                {
                    // Position
                    glm::vec3 position(
                        attractors[i]
                        + glm::vec3(rnd_dist(rnd_engine), rnd_dist(rnd_engine), rnd_dist(rnd_engine)) * 0.75F);
                    float len = glm::length(glm::normalize(position - attractors[i]));
                    position.y *= 2.0F - (len * len);

                    // Velocity
                    glm::vec3 angular = glm::vec3(0.5F, 1.5F, 0.5F) * (((i % 2) == 0) ? 1.0F : -1.0F);
                    glm::vec3 velocity =
                        glm::cross((position - attractors[i]), angular)
                        + glm::vec3(rnd_dist(rnd_engine), rnd_dist(rnd_engine), rnd_dist(rnd_engine) * 0.025F);

                    float mass    = (rnd_dist(rnd_engine) * 0.5F + 0.5F) * 75.0F;
                    body.position = body.last_position = glm::vec3(position);
                    body.velocity                      = glm::vec4(velocity, 0.0F);
                    body.mass                          = mass;
                }
            }
        }
    }

    void WriteComputeDescriptorSet()
    {
        VkDescriptorBufferInfo buffer_info =
            dhh::vk::initializer::descriptorBufferInfo(computeBuffer_.buffer, 0, VK_WHOLE_SIZE);

        VkWriteDescriptorSet write = {};
        write.descriptorCount      = 1;
        write.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pBufferInfo          = &buffer_info;
        write.dstSet               = comput_pipe->descriptorSets[0];
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        write.dstSet = cach_pipe->descriptorSets[0];
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    void Compute()
    {
        VkFence complete_fence;
        VkFence sdf;

        VkFenceCreateInfo fence_create_info = dhh::vk::initializer::fenceCreateInfo();
        vkCreateFence(device, &fence_create_info, nullptr, &complete_fence);

        VkSubmitInfo submit_info       = {};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &compute_cmd_buf;

        auto now = std::chrono::high_resolution_clock::now();
        vkQueueSubmit(graphicsQueue, 1, &submit_info, complete_fence);
        vkWaitForFences(device, 1, &complete_fence, true, UINT64_MAX);
        vkResetFences(device, 1, &complete_fence);

        // std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
        // std::chrono::high_resolution_clock::now() - now).count() << std::endl;

        void* data;
        vmaMapMemory(allocator, computeBuffer_.memory, &data);
        std::vector<Body> fuck(BODIES_COUNT);
        memcpy(fuck.data(), data, sizeof(Body) * BODIES_COUNT);
        vmaUnmapMemory(allocator, computeBuffer_.memory);

        std::cout << glm::to_string(fuck[5000].position) << "\n";
    }

    VkCommandBuffer compute_cmd_buf;

    void BuildComputeCommandBuffers()
    {
        VkCommandBufferAllocateInfo info =
            dhh::vk::initializer::commandBufferAllocateInfo(commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        vkAllocateCommandBuffers(device, &info, &compute_cmd_buf);
        VkCommandBufferBeginInfo begin_info =
            dhh::vk::initializer::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        vkBeginCommandBuffer(compute_cmd_buf, &begin_info);

        // calculate

        vkCmdBindPipeline(compute_cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, comput_pipe->pipeline);
        vkCmdBindDescriptorSets(compute_cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, comput_pipe->pipelineLayout, 0, 1,
            comput_pipe->descriptorSets.data(), 0, nullptr);
        vkCmdDispatch(compute_cmd_buf, BODIES_COUNT / 256, 1, 1);

        VkBufferMemoryBarrier barrier = {};
        barrier.buffer                = computeBuffer_.buffer;
        barrier.sType                 = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.size                  = VK_WHOLE_SIZE;
        barrier.srcAccessMask         = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask         = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(compute_cmd_buf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_NULL_HANDLE, 0, nullptr, 1, &barrier, VK_NULL_HANDLE, nullptr);

        // cache old data
        vkCmdBindPipeline(compute_cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, cach_pipe->pipeline);
        vkCmdBindDescriptorSets(compute_cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, cach_pipe->pipelineLayout, 0, 1,
            cach_pipe->descriptorSets.data(), 0, nullptr);
        vkCmdDispatch(compute_cmd_buf, BODIES_COUNT / 256, 1, 1);

        vkEndCommandBuffer(compute_cmd_buf);
    }


    void CreateComputeBuffer()
    {
        createBuffer(sizeof(Body) * bodies.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
            computeBuffer_.buffer, computeBuffer_.memory);
        void* data;
        vmaMapMemory(allocator, computeBuffer_.memory, &data);
        memcpy(data, bodies.data(), sizeof(Body) * bodies.size());
        vmaUnmapMemory(allocator, computeBuffer_.memory);
    }

    void CreateVertexBuffer()
    {
        createBuffer(sizeof(glm::vec3) * 2 * BODIES_COUNT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
            vertices_.buffer, vertices_.memory);
        positions.resize(BODIES_COUNT);
    }

    void CreateComputePipeline()
    {
        std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

        dhh::shader::Shader compute_shader(shaders_directory / "nbody.comp");
        comput_pipe = new dhh::shader::Pipeline(device, {&compute_shader}, descriptorPool);

        dhh::shader::Shader cache_shader(shaders_directory / "cache.comp");
        cach_pipe = new dhh::shader::Pipeline(device, {&cache_shader}, descriptorPool);
    }

    void CreateTrianglePipeline()
    {
        std::filesystem::path shaders_directory = dhh::shader::findShaderDirectory();

        dhh::shader::Shader vertex_shader(shaders_directory / "shader.vert");
        dhh::shader::Shader fragment_shader(shaders_directory / "shader.frag");

        triangle_pipe = new dhh::shader::Pipeline(device, {&vertex_shader, &fragment_shader}, descriptorPool,
            renderPass, dhh::vk::initializer::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT),
            {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR},
            dhh::vk::initializer::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_POINT, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE),
            dhh::vk::initializer::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS),
            dhh::vk::initializer::pipelineViewportStateCreateInfo(1, 1),
            dhh::vk::initializer::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                                                        | VK_COLOR_COMPONENT_B_BIT
                                                                        | VK_COLOR_COMPONENT_A_BIT,
                false),
            dhh::vk::initializer::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST));
    }


    void BuildCommandBuffers()
    {
        VkCommandBufferBeginInfo cmd_buf_info = dhh::vk::initializer::commandBufferBeginInfo();

        // Set clear values for all framebuffer attachments with loadOp set to clear
        // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to
        // set clear values for both
        std::vector<VkClearValue> clear_values(2);
        clear_values[0].color        = {{0.0F, 0.0F, 0.2F, 1.0F}};
        clear_values[1].depthStencil = {1.0F, 0};

        for (int32_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkRenderPassBeginInfo render_pass_begin_info = dhh::vk::initializer::renderPassBeginInfo(
                clear_values, framebuffers[i], renderPass, windowWidth, windowHeight);

            vkBeginCommandBuffer(commandBuffers[i], &cmd_buf_info);

            // Start the first sub pass specified in our default render pass setup by the base class
            // This will clear the color and depth attachment
            vkCmdBeginRenderPass(commandBuffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            // Update dynamic viewport state
            VkViewport viewport = {};
            viewport.height     = -static_cast<float>(windowHeight);
            viewport.width      = static_cast<float>(windowWidth);
            viewport.minDepth   = static_cast<float>(0.0F);
            viewport.maxDepth   = static_cast<float>(1.0F);
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
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipe->pipelineLayout,
                0, 1, &triangle_pipe->descriptorSets[0], 0, nullptr);

            // Bind the rendering pipeline
            // The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the
            // states specified at pipeline creation time
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipe->pipeline);

            // Bind triangle vertex buffer (contains position and colors)
            VkDeviceSize offsets[1] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertices_.buffer, offsets);

            vkCmdDraw(commandBuffers[i], BODIES_COUNT, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            // Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
            // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

            vkEndCommandBuffer(commandBuffers[i]);
        }
    }

    bool color_set = false;

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
    };

    std::vector<Vertex> positions;

    void UpdateVertexBuffer()
    {
        void* data;
        vmaMapMemory(allocator, computeBuffer_.memory, &data);
        std::vector<Body> fuck(BODIES_COUNT);
        memcpy(fuck.data(), data, sizeof(Body) * BODIES_COUNT);
        vmaUnmapMemory(allocator, computeBuffer_.memory);
        std::default_random_engine rnd_engine;
        std::normal_distribution<float> rnd_dist(0.0F, 1.0F);


        for (int i = 0; i < BODIES_COUNT; ++i)
        {
            positions[i].pos = fuck[i].position;
            if (!color_set)
            {
                positions[i].color = glm::vec3(rnd_dist(rnd_engine), rnd_dist(rnd_engine), rnd_dist(rnd_engine));
            }
        }
        std::cout << glm::to_string(positions[6000].pos) << "\n";
        color_set = true;

        vmaMapMemory(allocator, vertices_.memory, &data);
        memcpy(data, positions.data(), sizeof(Vertex) * BODIES_COUNT);
        vmaUnmapMemory(allocator, vertices_.memory);
    }
};


int main(int argc, char* argv[])
{
    try
    {
        Triangle app;

        int anchor   = 0;
        double years = 0;
        while (glfwWindowShouldClose(app.window) != GLFW_TRUE)
        {
            app.UpdateTransform();
            app.UpdateVertexBuffer();
            app.drawFrame();
            app.Compute();
            glfwPollEvents();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
