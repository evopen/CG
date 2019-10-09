#include "VulkanDebug.h"

VkBool32 engine::debug::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                      void* pUserData)
{
	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl << std::endl;
	return VK_FALSE;
}
