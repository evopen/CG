#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <string>

namespace engine
{
	namespace tool
	{
		VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags,
		                            uint32_t mipLevels);
		void createImage(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t mipLevelCount,
		                 VkSampleCountFlagBits sampleCount,
		                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
		                 VkImage& image, VmaAllocation& allocation);
		VkShaderModule createShaderModule(VkDevice device, const std::string& filename);
		void createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
		                  VmaMemoryUsage memoryUsage, VkBuffer& buffer,
		                  VmaAllocation& allocation);
		void createVertexInputBuffer(VmaAllocator allocator, VkDeviceSize size, bool staging, VkBuffer & buffer, VmaAllocation allocation,
		                        void* data);
		void createIndexInputBuffer(VmaAllocator allocator, VkDeviceSize size, bool staging, VkBuffer & buffer, VmaAllocation allocation,
		                        void* data);
	}
}
