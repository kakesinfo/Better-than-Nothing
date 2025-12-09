#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CShaderLoader final : public IResourceLoader<ShaderData, ShaderGpuData>
	{
	public:
		explicit CShaderLoader(VulkanContext* context);
		~CShaderLoader() override;

		std::unique_ptr<ShaderGpuData> Load(const ShaderData& input) override;

	private:
		static std::vector<char>	s_readFile(const std::string& filename);
		static glslang_stage_t		s_getShaderStage(EVulkanShaderType type);
		static glslang_program_t*	s_getShaderProgram(const std::vector<char>& source, glslang_stage_t stage);
		VkShaderModule				_getShaderModule(glslang_program_t* program) const;
	};
}
