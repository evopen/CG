#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace engine
{
	namespace debug
	{
		VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		                                   VkDebugUtilsMessageTypeFlagsEXT messageType,
		                                   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		                                   void* pUserData);

	}
}
