#pragma once
#include <glslang/Public/ShaderLang.h>
#include <vulkan/vulkan.h>
#include <unordered_map>

class Shader
{
public:
	std::vector<char> rawGlsl;
	std::vector<uint32_t> spirv;
	size_t attributeCount;
	std::vector<VkFormat> attributeFormats;
	VkShaderModule shaderModule;
	std::unordered_map<VkDescriptorType, uint32_t> descriptorCount;
	EShLanguage ShaderType;

private:
	                   glslang::TProgram program;

public:
	Shader(std::string filename);
	std::string getFileSuffix(const std::string& name);
	std::string getFilePath(const std::string& filename);
	EShLanguage getShaderStage(const std::string& stage);
	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(VkDevice device);
	void reflectAttributeFormats();
	void reflectDescriptors();
	static VkFormat getFormatFromGlType(int type);
};
