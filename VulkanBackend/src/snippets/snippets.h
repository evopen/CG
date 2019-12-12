#pragma once

#include <spirv_cross/spirv_cross.hpp>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <exception>
#include <map>
#include <shaderc/shaderc.hpp>
#include <Filesystem.hpp>
#include <Shader.hpp>

inline void compile_shader(const std::filesystem::path& shader_path)
{
	const std::filesystem::path spirv_directory = (shader_path.parent_path() / "spirv");
	if (!std::filesystem::exists(spirv_directory))
	{
		std::filesystem::create_directory(spirv_directory);
	}
	std::string shader_name = shader_path.filename().string();
	std::filesystem::path spirv_path = spirv_directory / (shader_name + ".spv");
	std::string command("glslc.exe \"" + shader_path.string() + "\" -o \"" + spirv_path.string() + "\"");
	system(command.c_str());
}

inline void compile_shaders(const std::filesystem::path& shader_directory)
{
	for (const auto& entry : std::filesystem::directory_iterator(shader_directory))
	{
		if (entry.is_regular_file())
		{
			compile_shader(entry.path().string());
		}
	}
}


inline VkFormat get_vk_format(spirv_cross::SPIRType type)
{
	VkFormat float_types[] = {
		VK_FORMAT_R32_SFLOAT,
		VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32B32A32_SFLOAT
	};
	switch (type.basetype)
	{
	case spirv_cross::SPIRType::Float:
		return float_types[type.vecsize - 1];
	default:
		throw std::exception("Cannot find VK_Format");
	}
}

inline std::vector<uint32_t> get_spirv_code(std::filesystem::path path)
{
	std::vector<char> GlslText = dhh::filesystem::loadFile(path, false);
	shaderc::Compiler Compiler;
	shaderc::CompileOptions Options;
	
	shaderc::SpvCompilationResult module = Compiler.CompileGlslToSpv(
		GlslText.data(), shaderc_glsl_infer_from_source, path.filename().string().c_str(), Options);
	if (module.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		throw std::exception("Failed to compile shader");
	}
	return {module.cbegin(), module.cend()};
}

inline VkVertexInputBindingDescription get_binding_description(const std::filesystem::path& vertex_spirv_path)
{
	spirv_cross::Compiler comp(get_spirv_code(vertex_spirv_path));
	spirv_cross::ShaderResources res = comp.get_shader_resources();

	uint32_t size = 0;
	for (const spirv_cross::Resource& resource : res.stage_inputs)
	{
		spirv_cross::SPIRType type = comp.get_type(resource.type_id);
		size += 4 * type.vecsize;
	}

	VkVertexInputBindingDescription description;
	description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	description.stride = size;
	description.binding = 0;

	return description;
}

inline std::vector<VkVertexInputAttributeDescription> get_attribute_description(
	const std::filesystem::path& vertex_spirv_path)
{
	spirv_cross::Compiler comp(get_spirv_code(vertex_spirv_path));
	spirv_cross::ShaderResources res = comp.get_shader_resources();
	std::vector<VkVertexInputAttributeDescription> descriptions;

	uint32_t offset = 0;
	for (const spirv_cross::Resource& resource : res.stage_inputs)
	{
		spirv_cross::SPIRType type = comp.get_type(resource.type_id);
		VkVertexInputAttributeDescription attribute = {};
		attribute.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
		attribute.location = comp.get_decoration(resource.id, spv::DecorationLocation);
		attribute.format = get_vk_format(type);
		attribute.offset = offset;

		descriptions.push_back(attribute);
		offset += 4 * type.vecsize;
	}
	return descriptions;
}


inline VkShaderModule createShaderModule(const std::filesystem::path& filename, VkDevice device)
{
	const std::vector<char>& code = dhh::filesystem::loadFile(filename, true);
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}


inline VkShaderStageFlagBits getShaderType(const std::filesystem::path& filePath)
{
	const std::unordered_map<std::string, VkShaderStageFlagBits> table{
		{"vert", VK_SHADER_STAGE_VERTEX_BIT},
		{"frag", VK_SHADER_STAGE_FRAGMENT_BIT},
	};
	std::string fileName = filePath.filename().string();
	size_t firstExtPos = fileName.find_first_of('.');
	size_t secondExtPos = fileName.find_last_of('.');
	std::string type = fileName.substr(firstExtPos + 1, secondExtPos - firstExtPos - 1);
	return table.find(type)->second;
}


inline std::map<std::string, VkPipelineShaderStageCreateInfo> createShaderStages(VkDevice device)
{
	std::map<std::string, VkPipelineShaderStageCreateInfo> shaderStageInfo;
	for (const auto& entry : std::filesystem::directory_iterator(dhh::shader::findShaderDirectory() / "spirv"))
	{
		std::string shaderName = entry.path().filename().replace_extension("").string();

		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = getShaderType(entry);
		stageInfo.module = createShaderModule(entry, device);
		stageInfo.pName = "main";

		shaderStageInfo[shaderName] = stageInfo;
	}
	return shaderStageInfo;
}
