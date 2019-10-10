#include "VulkanBase.h"
#include "VulkanDebug.h"
#include <stdexcept>
#include <set>
#include "VulkanTool.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <array>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void VulkanBase::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, width / 2, height / 2);
}

void VulkanBase::initVulkan()
{
	createInstance();
	if (bEnableValidation)
	{
		setupDebugMessenger();
	}
	pickPhysicalDevice();
	findQueueFamilyIndex();
	createLogicalDevice();
	createMemoryAllocator();
	chooseImageFormat();
	createSwapchain();
	createSwapchainImageViews();
	createRenderPass();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createCommandPool();
	createSyncObjects();
	allocateCommandBuffers();
}

void VulkanBase::loop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		processInput();
		draw();
	}
}

void VulkanBase::createInstance()
{
	VkApplicationInfo appCreateInfo = {};
	appCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appCreateInfo.apiVersion = VK_API_VERSION_1_1;
	appCreateInfo.pApplicationName = title.c_str();
	appCreateInfo.pEngineName = "NO ENGINE";
	appCreateInfo.pNext = nullptr;

	const std::vector<const char*> requiredExtensions = getRequiredExtensions();
	const std::vector<const char*> requiredLayers = getRequiredLayers();

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = requiredLayers.data();
	instanceCreateInfo.pApplicationInfo = &appCreateInfo;
	instanceCreateInfo.flags = VK_NULL_HANDLE;
	if (bEnableValidation)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;
		debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsMessengerCreateInfo.pNext = nullptr;
		debugUtilsMessengerCreateInfo.flags = VK_NULL_HANDLE;
		debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugUtilsMessengerCreateInfo.pUserData = nullptr;
		debugUtilsMessengerCreateInfo.pfnUserCallback = &engine::debug::debugCallback;
		instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
	}
	else
	{
		instanceCreateInfo.pNext = nullptr;
	}
	vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

void VulkanBase::setupDebugMessenger()
{
	auto CreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
	debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsMessengerCreateInfo.pfnUserCallback = &engine::debug::debugCallback;

	if (CreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger");
	}
}

void VulkanBase::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		std::cout << properties.deviceName << properties.deviceType << "\n";
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = device;
			return;
		}
	}
	throw std::runtime_error("failed to find discrete gpu");
}

void VulkanBase::findQueueFamilyIndex()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	/// Find Graphics queue family index
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndex.graphicsFamily = static_cast<uint32_t>(i);
			break;
		}
	}

	// Get window surface from GLFW
	glfwCreateWindowSurface(instance, window, nullptr, &surface);

	/// Find queue family that support presentation
	VkBool32 presentSupport;
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, static_cast<uint32_t>(i), surface, &presentSupport);
		if (presentSupport)
		{
			queueFamilyIndex.presentFamily = static_cast<uint32_t>(i);
			break;
		}
	}

	/// Find Transfer queue family index
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndex.transferFamily = static_cast<uint32_t>(i);
			break;
		}
	}

	if (!queueFamilyIndex.isComplete())
	{
		throw std::runtime_error("queue family incomplete");
	}
}

void VulkanBase::createLogicalDevice()
{
	float queuePriority = 1.f;
	std::set<uint32_t> uniqueQueueFamily = {
		queueFamilyIndex.graphicsFamily.value(),
		queueFamilyIndex.presentFamily.value(),
		queueFamilyIndex.transferFamily.value()
	};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (uint32_t queueIndex : uniqueQueueFamily)
	{
		VkDeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.flags = VK_NULL_HANDLE;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = queueIndex;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.flags = VK_NULL_HANDLE;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.pNext = nullptr;

	vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

	vkGetDeviceQueue(device, queueFamilyIndex.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndex.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device, queueFamilyIndex.transferFamily.value(), 0, &transferQueue);
}

void VulkanBase::createMemoryAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;

	vmaCreateAllocator(&allocatorInfo, &allocator);
}

void VulkanBase::createSwapchain()
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	VkSwapchainCreateInfoKHR swapchainCreateInfo;
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = VK_NULL_HANDLE;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 3;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = VK_NULL_HANDLE;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = nullptr;

	vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
}

void VulkanBase::createSwapchainImageViews()
{
	swapchainImageViews.resize(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); i++)
	{
		swapchainImageViews[i] =
			engine::tool::createImageView(device, swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT,
			                              1);
	}
}

void VulkanBase::createRenderPass()
{
	VkAttachmentDescription colorAttachment;
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.flags = VK_NULL_HANDLE;
	colorAttachment.samples = sampleCount;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // DONT_CARE?
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentDescription depthAttachment;
	depthAttachment.format = depthImageFormat;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.flags = VK_NULL_HANDLE;
	depthAttachment.samples = sampleCount;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentDescription colorAttachmentResolve;
	colorAttachmentResolve.format = surfaceFormat.format;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachmentResolve.flags = VK_NULL_HANDLE;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef;
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef;
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = VK_NULL_HANDLE;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentRef;
	subpassDescription.pResolveAttachments = &colorAttachmentResolveRef;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;
	renderPassCreateInfo.flags = VK_NULL_HANDLE;
	renderPassCreateInfo.pNext = nullptr;

	vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
}

void VulkanBase::createColorResources()
{
	engine::tool::createImage(allocator, width, height, 1, sampleCount, surfaceFormat.format,
	                          VK_IMAGE_TILING_OPTIMAL,
	                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	                          VMA_MEMORY_USAGE_GPU_ONLY,
	                          sColorImage.image, sColorImage.allocation);
	sColorImage.view = engine::tool::createImageView(device, sColorImage.image, surfaceFormat.format,
	                                                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanBase::createDepthResources()
{
	engine::tool::createImage(allocator, width, height, 1, sampleCount, depthImageFormat, VK_IMAGE_TILING_OPTIMAL,
	                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
	                          VMA_MEMORY_USAGE_GPU_ONLY,
	                          sDepthImage.image, sDepthImage.allocation);
	sDepthImage.view = engine::tool::createImageView(device, sDepthImage.image, depthImageFormat,
	                                                 VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void VulkanBase::createFramebuffers()
{
	swapchainFramebuffers.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = {
			sColorImage.view,
			sDepthImage.view,
			swapchainImageViews[i],
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanBase::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics command pool!");
	}
}

void VulkanBase::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanBase::allocateCommandBuffers()
{
	commandBuffers.resize(swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
}

std::vector<const char*> VulkanBase::getRequiredExtensions()
{
	uint32_t extensionCount = 0;
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> requiredExtensions(glfwRequiredExtensions, glfwRequiredExtensions + extensionCount);

	if (bEnableValidation)
	{
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return requiredExtensions;
}

std::vector<const char*> VulkanBase::getRequiredLayers()
{
	std::vector<const char*> requiredLayers(extraLayers);
	if (bEnableValidation)
	{
		requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
	}
	return requiredLayers;
}

void VulkanBase::chooseImageFormat()
{
	surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	depthImageFormat = VK_FORMAT_D32_SFLOAT;
}

void VulkanBase::draw()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
	                      &imageIndex);
	updateData(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanBase::processInput()
{
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	float cameraSpeed = 50.f * deltaTime;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos += glm::vec3(0, 1, 0) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		cameraPos += glm::vec3(0, -1, 0) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		exit(0);

	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top
	lastX = xPos;
	lastY = yPos;

	float sensitivity = 0.1f; // change this value to your liking
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
