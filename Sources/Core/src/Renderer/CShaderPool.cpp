//
// Created by acoezard on 11/19/24.
//

#include "CShaderPool.hpp"

#include "CWindowBuilder.hpp"
#include "Renderer/Vulkan/EVulkanShaderType.hpp"
#include "Renderer/Vulkan/CVulkanShaderModule.hpp"

#include <utility>

namespace BetterThanNothing
{
	CShaderPool::CShaderPool(std::string basePath, CVulkanContext* context)
		: m_context(context), m_basePath(std::move(basePath))
	{
		glslang_initialize_process();
	}

	CShaderPool::~CShaderPool()
	{
		if (m_context && m_context->vkDevice)
		{
			const auto device = m_context->vkDevice->LogicalHandle();
			for (const auto &val: m_shaders | std::views::values)
				vkDestroyShaderModule(device, val.shaderModule, nullptr);
		}

		glslang_finalize_process();
	}

	const CVulkanShaderModule& CShaderPool::LoadShader(const std::string& path, const EVulkanShaderType type)
	{
		const std::string fullPath = m_basePath + path;

		const auto it = m_shaders.find(fullPath);
		if (it != m_shaders.end())
			return it->second;

		const std::vector<char> shaderSource = _readFile(fullPath);
		const glslang_stage_t shaderStage = _getShaderStage(type);
		glslang_program_t* shaderProgram = _getShaderProgram(shaderSource, shaderStage);

		m_shaders[fullPath].shaderPath = path;
		m_shaders[fullPath].shaderType = type;
		m_shaders[fullPath].shaderModule = _getShaderModule(shaderProgram);

		LOG_SUCCESS("Shader loaded: " << path);

		return m_shaders[fullPath];
	}

	std::vector<char> CShaderPool::_readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		std::streampos endPos = file.tellg();
		if (endPos < 0)
			throw std::runtime_error("failed to determine file size!");

		const size_t fileSize = endPos;

		std::vector<char> buffer(fileSize + 1);
		if (fileSize > 0)
		{
			file.seekg(0, std::ios::beg);
			file.read(buffer.data(), fileSize);
			if (!file)
				throw std::runtime_error("failed to read file!");
		}

		buffer[fileSize] = '\0';
		file.close();

		return buffer;
	}

	glslang_stage_t CShaderPool::_getShaderStage(EVulkanShaderType type)
	{
		switch (type)
		{
			case EVulkanShaderType::Vertex:
				return GLSLANG_STAGE_VERTEX;
			case EVulkanShaderType::Fragment:
				return GLSLANG_STAGE_FRAGMENT;
			default:
				throw std::runtime_error("Invalid shader type");
		}
	}

	glslang_program_t* CShaderPool::_getShaderProgram(const std::vector<char>& source, glslang_stage_t stage)
	{
		const glslang_input_t input = {
			.language = GLSLANG_SOURCE_GLSL,
			.stage = stage,
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_3,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_0,
			.code = source.data(),
			.default_version = 450,
			.default_profile = GLSLANG_NO_PROFILE,
			.force_default_version_and_profile = false,
			.forward_compatible = false,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = glslang_default_resource(),
			.callbacks = {},
			.callbacks_ctx = nullptr
		};

		using shader_ptr_t = std::unique_ptr<glslang_shader_t, decltype(&glslang_shader_delete)>;
		shader_ptr_t shader(glslang_shader_create(&input), glslang_shader_delete);

		if (!glslang_shader_preprocess(shader.get(), &input))
			throw std::runtime_error("failed to preprocess shader: " + std::string(glslang_shader_get_info_log(shader.get())));

		if (!glslang_shader_parse(shader.get(), &input))
			throw std::runtime_error("failed to parse shader: " + std::string(glslang_shader_get_info_log(shader.get())));

		glslang_program_t* program = glslang_program_create();
		glslang_program_add_shader(program, shader.get());

		if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
			throw std::runtime_error("failed to compile shader program: " + std::string(glslang_program_get_info_log(program)));

		glslang_program_SPIRV_generate(program, input.stage);

		if (glslang_program_SPIRV_get_messages(program))
			LOG_INFO("Shader messages: " << glslang_program_SPIRV_get_messages(program));

		{
			const uint32* spirvCode = glslang_program_SPIRV_get_ptr(program);
			const size_t spirvSize = glslang_program_SPIRV_get_size(program);

			const spirv_cross::Compiler compiler(spirvCode, spirvSize);
			const spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

			if (!shaderResources.stage_inputs.empty())
			{
				for (const auto& resource : shaderResources.stage_inputs)
					LOG_INFO("Stage input: " << resource.name);
			}

			if (!shaderResources.stage_outputs.empty())
			{
				for (const auto& resource : shaderResources.stage_outputs)
					LOG_INFO("Stage output: " << resource.name);
			}

			if (!shaderResources.uniform_buffers.empty())
				LOG_INFO("Uniform buffers: " << shaderResources.uniform_buffers.size());

			if (!shaderResources.storage_buffers.empty())
				LOG_INFO("Storage buffers: " << shaderResources.storage_buffers.size());

			if (!shaderResources.sampled_images.empty())
				LOG_INFO("Sampled images: " << shaderResources.sampled_images.size());
		}

		return program;
	}

	VkShaderModule CShaderPool::_getShaderModule(glslang_program_t* program) const
	{
		if (!m_context || !m_context->vkDevice)
			throw std::runtime_error("Invalid Vulkan context when creating shader module");

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);
		createInfo.pCode = glslang_program_SPIRV_get_ptr(program);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_context->vkDevice->LogicalHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module!");

		glslang_program_delete(program);

		return shaderModule;
	}

};
