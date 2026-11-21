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

#include <Common/Logger.hpp>
#include <ResourceImporter/Importer.hpp>
#include <ResourceImporter/ResourceImporter_Types.hpp>
#include <filesystem>
#include <fstream>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/resource_limits_c.h>
#include <memory>
#include <string>

namespace BetterThanNothing
{
	static EShaderType Internal_GetShaderType(const std::string& path)
	{
		const std::string ext = std::filesystem::path(path).extension().string();
		if (ext == ".vert")
			return EShaderType::Vertex;
		if (ext == ".frag")
			return EShaderType::Fragment;
		if (ext == ".geom")
			return EShaderType::Geometry;
		if (ext == ".comp")
			return EShaderType::Compute;
		throw std::runtime_error("Unknown shader type for: " + path);
	}

	static glslang_stage_t Internal_GetGlslangStage(const EShaderType type)
	{
		switch (type)
		{
			case EShaderType::Vertex:
				return GLSLANG_STAGE_VERTEX;
			case EShaderType::Fragment:
				return GLSLANG_STAGE_FRAGMENT;
			case EShaderType::Geometry:
				return GLSLANG_STAGE_GEOMETRY;
			case EShaderType::Compute:
				return GLSLANG_STAGE_COMPUTE;
			default:
				throw std::runtime_error("Invalid shader type");
		}
	}

	ImportedShader ShaderImporter::Import(const std::string& input)
	{
		std::ifstream file(input, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open shader file: " + input);

		const std::streampos endPos = file.tellg();
		if (endPos < 0)
			throw std::runtime_error("Failed to determine shader file size: " + input);

		const auto	fileSize = static_cast<size_t>(endPos);
		std::string source(fileSize, '\0');

		if (fileSize > 0)
		{
			file.seekg(0, std::ios::beg);
			file.read(source.data(), static_cast<std::streamsize>(fileSize));
			if (!file)
				throw std::runtime_error("Failed to read shader file: " + input);
		}
		file.close();

		const EShaderType shaderType = Internal_GetShaderType(input);

		const glslang_input_t glslInput = {
			.language = GLSLANG_SOURCE_GLSL,
			.stage = Internal_GetGlslangStage(shaderType),
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_3,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_6,
			.code = source.c_str(),
			.default_version = 450,
			.default_profile = GLSLANG_NO_PROFILE,
			.force_default_version_and_profile = false,
			.forward_compatible = false,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = glslang_default_resource(),
			.callbacks = {},
			.callbacks_ctx = nullptr,
		};

		using ShaderPtr = std::unique_ptr<glslang_shader_t, decltype(&glslang_shader_delete)>;
		using ProgramPtr = std::unique_ptr<glslang_program_t, decltype(&glslang_program_delete)>;

		const ShaderPtr shader(glslang_shader_create(&glslInput), glslang_shader_delete);
		if (!shader)
			throw std::runtime_error("Failed to allocate glslang shader for: " + input);

		if (!glslang_shader_preprocess(shader.get(), &glslInput))
			throw std::runtime_error("Failed to preprocess shader (" + input + "): " + std::string(glslang_shader_get_info_log(shader.get())));

		if (!glslang_shader_parse(shader.get(), &glslInput))
			throw std::runtime_error("Failed to parse shader (" + input + "): " + std::string(glslang_shader_get_info_log(shader.get())));

		const ProgramPtr program(glslang_program_create(), glslang_program_delete);
		if (!program)
			throw std::runtime_error("Failed to allocate glslang program for: " + input);

		glslang_program_add_shader(program.get(), shader.get());

		if (!glslang_program_link(program.get(), GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
			throw std::runtime_error("Failed to link shader program (" + input + "): " + std::string(glslang_program_get_info_log(program.get())));

		glslang_program_SPIRV_generate(program.get(), glslInput.stage);

		if (const char* messages = glslang_program_SPIRV_get_messages(program.get()); messages && messages[0] != '\0')
			Logger::Log<WARNING>("Shader SPIR-V messages ({}): {}", input, messages);

		const size_t	wordCount = glslang_program_SPIRV_get_size(program.get());
		const uint32_t* words = glslang_program_SPIRV_get_ptr(program.get());

		ImportedShader importedShader;
		importedShader.path = input;
		importedShader.type = shaderType;
		importedShader.spirv.assign(words, words + wordCount);

		return importedShader;
	}
}
