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

#pragma once

#include "ResourceLoader.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;
	struct ShaderGpuData;
	class VulkanShaderModule;
	enum class EShaderType;

	struct ShaderData
	{
		EShaderType type;
		std::string filePath;
	};

	class ShaderLoader final : public ResourceLoader<ShaderData, ShaderGpuData>
	{
	public:
		explicit ShaderLoader(VulkanContext* context);
		~ShaderLoader() override;

		std::unique_ptr<ShaderGpuData> Load(const ShaderData& input) override;

	private:
		static std::vector<char>			ReadFile(const std::string& filename);
		static glslang_stage_t				GetShaderStage(EShaderType type);
		static glslang_program_t*			GetShaderProgram(const std::vector<char>& source, glslang_stage_t stage);
		std::unique_ptr<VulkanShaderModule> GetShaderModule(glslang_program_t* program) const;
	};
}
