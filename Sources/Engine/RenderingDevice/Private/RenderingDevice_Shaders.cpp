/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "Common/Logger.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice/ShaderDescription.hpp"
#include "RenderingDevice_Internals.hpp"

#include <RenderingApi.hpp>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_cross/spirv_cross.hpp>

namespace BetterThanNothing
{
	static glslang_stage_t Internal_GetShaderStage(const EShaderType type)
	{
		switch (type)
		{
			case EShaderType::Vertex:
				return GLSLANG_STAGE_VERTEX;
			case EShaderType::Fragment:
				return GLSLANG_STAGE_FRAGMENT;
			default:
				throw std::runtime_error("Invalid shader type");
		}
	}

	static bool Internal_CreateShader(RenderingApi::Shader* dstShader, const ShaderDescription& desc, const RenderingApi::Device* device)
	{
		const glslang_input_t input =
		{.language = GLSLANG_SOURCE_GLSL,
		 .stage = Internal_GetShaderStage(desc.type),
		 .client = GLSLANG_CLIENT_VULKAN,
		 .client_version = GLSLANG_TARGET_VULKAN_1_3,
		 .target_language = GLSLANG_TARGET_SPV,
		 .target_language_version = GLSLANG_TARGET_SPV_1_0,
		 .code = desc.data,
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

		glslang_program_t* shaderProgram = glslang_program_create();
		glslang_program_add_shader(shaderProgram, shader.get());

		if (!glslang_program_link(shaderProgram, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
			throw std::runtime_error("failed to compile shader program: " + std::string(glslang_program_get_info_log(shaderProgram)));

		glslang_program_SPIRV_generate(shaderProgram, input.stage);

		if (glslang_program_SPIRV_get_messages(shaderProgram))
			Logger::Log<WARNING>("Shader warnings: {}", glslang_program_SPIRV_get_messages(shaderProgram));

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = glslang_program_SPIRV_get_size(shaderProgram) * sizeof(unsigned int);
		createInfo.pCode = glslang_program_SPIRV_get_ptr(shaderProgram);

		if (device->CreateShaderModule(createInfo, &dstShader->module) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module!");

		dstShader->type = desc.type;
		dstShader->name = desc.name;

		glslang_program_delete(shaderProgram);

		return true;
	}


	Handle RenderingDevice::CreateShader(const ShaderDescription& desc)
	{
		const auto handle = m_self->shaders.Allocate();

		if (!Internal_CreateShader(m_self->shaders.Get(handle), desc, m_self->device.get()))
			throw std::runtime_error("Failed to create shader");

		return handle;
	}

	void RenderingDevice::DestroyShader(const Handle& handle)
	{
		if (!m_self->shaders.Has(handle))
			throw std::runtime_error("Invalid shader handle!");

		const RenderingApi::Shader* shader = m_self->shaders.Get(handle);
		m_self->device->DestroyShaderModule(shader->module);
		m_self->shaders.Destroy(handle);
	}
}
