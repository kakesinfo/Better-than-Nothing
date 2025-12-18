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

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>
#include <spirv_cross/spirv_cross.hpp>

#include "Module_Renderer/Resources/IResourceLoader.hpp"
#include "Module_Renderer/Data/ShaderData.hpp"
#include "Module_Vulkan/Data/ShaderGpuData.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;
	class CVulkanShaderModule;
	enum class EVulkanShaderType;

	class CShaderLoader final : public IResourceLoader<ShaderData, ShaderGpuData>
	{
	public:
		explicit CShaderLoader(VulkanContext* context);
		~CShaderLoader() override;

		std::unique_ptr<ShaderGpuData> Load(const ShaderData& input) override;

	private:
		static std::vector<char>				s_readFile(const std::string& filename);
		static glslang_stage_t					s_getShaderStage(EVulkanShaderType type);
		static glslang_program_t*				s_getShaderProgram(const std::vector<char>& source, glslang_stage_t stage);
		std::unique_ptr<CVulkanShaderModule>	_getShaderModule(glslang_program_t* program) const;
	};
}
