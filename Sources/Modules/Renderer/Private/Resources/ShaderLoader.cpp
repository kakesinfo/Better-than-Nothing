/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Module_Renderer/Resources/ShaderLoader.hpp"
#include "Module_Renderer/Data/ShaderData.hpp"

#include "Module_Vulkan/Data/ShaderGpuData.hpp"
#include "Module_Vulkan/EVulkanShaderType.hpp"
#include "Module_Vulkan/VulkanContext.hpp"
#include "Module_Vulkan/VulkanShaderModule.hpp"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_cross/spirv_cross.hpp>

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	ShaderLoader::ShaderLoader(VulkanContext* context) : ResourceLoader(context)
	{
		glslang_initialize_process();
	}

	ShaderLoader::~ShaderLoader()
	{
		glslang_finalize_process();
	}

	std::unique_ptr<ShaderGpuData> ShaderLoader::Load(const ShaderData& input)
	{
		auto shaderGpuData = std::make_unique<ShaderGpuData>();

		const std::vector<char> shaderSource = ReadFile(input.filePath);
		const glslang_stage_t	shaderStage = GetShaderStage(input.type);
		glslang_program_t*		shaderProgram = GetShaderProgram(shaderSource, shaderStage);

		shaderGpuData->type = input.type;
		shaderGpuData->shaderModule = GetShaderModule(shaderProgram);

		glslang_program_delete(shaderProgram);

		return shaderGpuData;
	}

	std::vector<char> ShaderLoader::ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		const std::streampos endPos = file.tellg();
		if (endPos < 0)
			throw std::runtime_error("failed to determine file size!");

		const std::streamsize fileSize = endPos;

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

	glslang_stage_t ShaderLoader::GetShaderStage(const EVulkanShaderType type)
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

	glslang_program_t* ShaderLoader::GetShaderProgram(const std::vector<char>& source, glslang_stage_t stage)
	{
		const glslang_input_t input = {.language = GLSLANG_SOURCE_GLSL,
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
									   .callbacks_ctx = nullptr};

		using shader_ptr_t = std::unique_ptr<glslang_shader_t, decltype(&glslang_shader_delete)>;
		const shader_ptr_t shader(glslang_shader_create(&input), glslang_shader_delete);

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
			Logger::Log<WARNING>("Shader warnings: {}", glslang_program_SPIRV_get_messages(program));

		// {
		// 	const uint32 *spirvCode = glslang_program_SPIRV_get_ptr(program);
		// 	const size_t spirvSize = glslang_program_SPIRV_get_size(program);
		//
		// 	const spirv_cross::Compiler compiler(spirvCode, spirvSize);
		// 	const spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();
		//
		// 	if (!shaderResources.stage_inputs.empty()) {
		// 		for (const auto &resource: shaderResources.stage_inputs)
		// 			LOG_INFO("Stage input: " << resource.name);
		// 	}
		//
		// 	if (!shaderResources.stage_outputs.empty()) {
		// 		for (const auto &resource: shaderResources.stage_outputs)
		// 			LOG_INFO("Stage output: " << resource.name);
		// 	}
		//
		// 	if (!shaderResources.uniform_buffers.empty())
		// 		LOG_INFO("Uniform buffers: " << shaderResources.uniform_buffers.size());
		//
		// 	if (!shaderResources.storage_buffers.empty())
		// 		LOG_INFO("Storage buffers: " << shaderResources.storage_buffers.size());
		//
		// 	if (!shaderResources.sampled_images.empty())
		// 		LOG_INFO("Sampled images: " << shaderResources.sampled_images.size());
		// }

		return program;
	}

	std::unique_ptr<VulkanShaderModule> ShaderLoader::GetShaderModule(glslang_program_t* program) const
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);
		createInfo.pCode = glslang_program_SPIRV_get_ptr(program);

		return std::make_unique<VulkanShaderModule>(m_context, &createInfo);
	}
}
