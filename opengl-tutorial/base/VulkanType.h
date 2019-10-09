#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>

namespace engine
{
	namespace type
	{
		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};


		struct QueueFamilyIndex
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily;

			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
			}
		};

		struct Image
		{
			VkImage image;
			VkImageView view;
			VmaAllocation allocation;
		};

		struct Buffer
		{
			VkBuffer buffer;
			VmaAllocation allocation;
		};
	}
}
