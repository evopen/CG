#include "VulkanShader.h"
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <SPIRV/Logger.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/gl_types.h>

TBuiltInResource GetResources()
{
	TBuiltInResource resources = {};
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = 32;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = 8;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.limits.nonInductiveForLoops = true;
	resources.limits.whileLoops = true;
	resources.limits.doWhileLoops = true;
	resources.limits.generalUniformIndexing = true;
	resources.limits.generalAttributeMatrixVectorIndexing = true;
	resources.limits.generalVaryingIndexing = true;
	resources.limits.generalSamplerIndexing = true;
	resources.limits.generalVariableIndexing = true;
	resources.limits.generalConstantMatrixVectorIndexing = true;
	return resources;
}

Shader::Shader(std::string filename)
{
	glslang::InitializeProcess();

	ShaderType = getShaderStage(getFileSuffix(filename));
	glslang::TShader shader(ShaderType);

	rawGlsl = readFile(filename);
	const char* shaderArray[] = {rawGlsl.data()};
	shader.setStrings(shaderArray, 1);

	int ClientInputSemanticsVersion = 110;
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_3;

	shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);
	TBuiltInResource resource = GetResources();
	EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

	if (!shader.parse(&resource, ClientInputSemanticsVersion, false, messages))
	{
		std::cout << "GLSL Parsing Failed for: " << filename << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
	}

	program.addShader(&shader);
	if (!program.link(messages) || !program.mapIO())
	{
		std::cout << "GLSL Linking Failed for: " << filename << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
	}
	program.buildReflection();
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = false;
	spvOptions.generateDebugInfo = false;
	spvOptions.optimizeSize = false;
	spvOptions.validate = false;
	glslang::GlslangToSpv(*program.getIntermediate(ShaderType), spirv, &logger, &spvOptions);

	reflectAttributeFormats();
	reflectDescriptors();

	glslang::FinalizeProcess();
}

std::string Shader::getFileSuffix(const std::string& name)
{
	const size_t pos = name.rfind('.');
	return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}

std::string Shader::getFilePath(const std::string& filename)
{
	size_t found = filename.find_last_of("/\\");
	return filename.substr(0, found);
}

EShLanguage Shader::getShaderStage(const std::string& stage)
{
	if (stage == "vert")
	{
		return EShLangVertex;
	}
	else if (stage == "tesc")
	{
		return EShLangTessControl;
	}
	else if (stage == "tese")
	{
		return EShLangTessEvaluation;
	}
	else if (stage == "geom")
	{
		return EShLangGeometry;
	}
	else if (stage == "frag")
	{
		return EShLangFragment;
	}
	else if (stage == "comp")
	{
		return EShLangCompute;
	}
	else
	{
		assert(0 && "Unknown shader stage");
		return EShLangCount;
	}
}

std::vector<char> Shader::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}
	size_t size = file.tellg();
	std::vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), size);

	return buffer;
}

VkShaderModule Shader::createShaderModule(VkDevice device)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = spirv.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


void Shader::reflectAttributeFormats()
{
	attributeCount = program.getNumPipeInputs();
	attributeFormats.resize(attributeCount);
	for (size_t i = 0; i < attributeCount; i++)
	{
		attributeFormats[i] = getFormatFromGlType(program.getAttributeType(i));
	}
}

void Shader::reflectDescriptors()
{
	if (program.getNumUniformBlocks() != 0)
	{
		descriptorCount.insert_or_assign(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, program.getNumUniformBlocks());
	}

	size_t samplerCount = 0;
	for (int i = 0; i < program.getNumUniformVariables(); i++)
	{
		if (program.getUniformType(i) == GL_SAMPLER_2D)
			samplerCount++;
	}
	descriptorCount.insert_or_assign(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, samplerCount);
}

VkFormat Shader::getFormatFromGlType(int type)
{
	switch (type)
	{
	case 0x1406: // GL_FLOAT
		return VK_FORMAT_R32_SFLOAT;
	case 0x8B50: // GL_FLOAT_VEC2
		return VK_FORMAT_R32G32_SFLOAT;
	case 0x8B51: // GL_FLOAT_VEC3
		return VK_FORMAT_R32G32B32_SFLOAT;
	case 0x8B52: // GL_FLOAT_VEC4
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case 0x1404: // GL_INT
		return VK_FORMAT_R32_SINT;
	case 0x8B53: // GL_INT_VEC2
		return VK_FORMAT_R32G32_SINT;
	case 0x8B54: // GL_INT_VEC3
		return VK_FORMAT_R32G32B32_SINT;
	case 0x8B55: // GL_INT_VEC4
		return VK_FORMAT_R32G32B32A32_SINT;
	case 0x1405: // GL_UNSIGNED_INT
		return VK_FORMAT_R32_SINT;
	case 0x8DC6: // GL_UNSIGNED_INT_VEC2
		return VK_FORMAT_R32G32_SINT;
	case 0x8DC7: // GL_UNSIGNED_INT_VEC3
		return VK_FORMAT_R32G32B32_SINT;
	case 0x8DC8: // GL_UNSIGNED_INT_VEC4
		return VK_FORMAT_R32G32B32A32_SINT;
	default:
		return VK_FORMAT_UNDEFINED;
	}
}
