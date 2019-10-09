#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <optional>
#include "VulkanType.h"


const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> extraLayers = {
        "VK_LAYER_LUNARG_monitor"
};

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanBase
{
public:
    explicit VulkanBase(bool enableValidation)
            : bEnableValidation(enableValidation)
    {
        initWindow();
        initVulkan();
    }

protected:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    engine::type::QueueFamilyIndex queueFamilyIndex;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VmaAllocator allocator;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkSurfaceFormatKHR surfaceFormat;
    VkFormat depthImageFormat;
    VkRenderPass renderPass;
    engine::type::Image sColorImage;
    engine::type::Image sDepthImage;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkCommandBuffer> commandBuffers;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBufferAllocation;

protected:
    bool bEnableValidation = true;
    uint32_t width = 800;
    uint32_t height = 600;
    std::string title = "Example";
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_8_BIT;
    uint32_t currentFrame = 0;

    /// camera stuff
    glm::vec3 cameraPos = glm::vec3(0, 5, 20);
    glm::vec3 cameraFront = glm::vec3(-1.0f, -1.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX = width / 2, lastY = height / 2;
    float yaw = -90.f, pitch = -10;
    ///


public:
    void loop();

private:
    /// main procedures
    void initWindow();
    void initVulkan();

    /// initVulkan() sub-procedure
    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void findQueueFamilyIndex();
    void createLogicalDevice();
    void createMemoryAllocator();
    void createSwapchain();
    void createSwapchainImageViews();
    void createRenderPass();
    void createColorResources();
    void createDepthResources();
    void createFramebuffers();
    void createCommandPool();
    void createSyncObjects();
    void allocateCommandBuffers();

    /// helper function
    std::vector<const char*> getRequiredExtensions();
    std::vector<const char*> getRequiredLayers();
    void chooseImageFormat();

    void draw();
    void processInput();
    virtual void updateData(uint32_t imageIndex) = 0;
};
