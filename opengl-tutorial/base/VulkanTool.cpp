#include "VulkanTool.h"
#include <vector>
#include <stdexcept>
#include <fstream>

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlagBits aspectFlags,
                            uint32_t mipLevels)
{
	VkImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.image = image;
	viewCreateInfo.flags = VK_NULL_HANDLE;
	viewCreateInfo.format = format;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.pNext = nullptr;

	VkImageView imageView;
	vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView);

	return imageView;
}

void createImage(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t mipLevelCount,
                 VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usage,
                 VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& allocation)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevelCount;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.flags = VK_NULL_HANDLE;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usage;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.queueFamilyIndexCount = VK_NULL_HANDLE;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.usage = memoryUsage;

	vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr);
}

VkImageView engine::tool::createImageView(VkDevice device, VkImage image, VkFormat format,
                                          VkImageAspectFlagBits aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void engine::tool::createImage(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t mipLevelCount,
                               VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VmaMemoryUsage memoryUsage, VkImage& image, VmaAllocation& allocation)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = mipLevelCount;
	imageCreateInfo.samples = sampleCount;
	imageCreateInfo.flags = VK_NULL_HANDLE;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usage;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.queueFamilyIndexCount = VK_NULL_HANDLE;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.usage = memoryUsage;

	vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr);
}

VkShaderModule engine::tool::createShaderModule(VkDevice device, const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void engine::tool::createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
                                VmaMemoryUsage memoryUsage,
                                VkBuffer& buffer, VmaAllocation& allocation)
{
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = VK_NULL_HANDLE;
	bufferCreateInfo.queueFamilyIndexCount = VK_NULL_HANDLE;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.usage = memoryUsage;

	vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);
}

void engine::tool::createVertexInputBuffer(VmaAllocator allocator, VkDeviceSize size, bool staging, VkBuffer& buffer,
                                           VmaAllocation allocation, void* data)
{
	createBuffer(allocator, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, buffer, allocation);

	void* p;
	vmaMapMemory(allocator, allocation, &p);
	memcpy(p, data, size);
	vmaUnmapMemory(allocator, allocation);
}

void engine::tool::createIndexInputBuffer(VmaAllocator allocator, VkDeviceSize size, bool staging, VkBuffer& buffer,
	VmaAllocation allocation, void* data)
{
	createBuffer(allocator, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, buffer, allocation);

	void* p;
	vmaMapMemory(allocator, allocation, &p);
	memcpy(p, data, size);
	vmaUnmapMemory(allocator, allocation);
}
