//
// Created by acoezard on 11/19/24.
//

#pragma once

#include "../Renderer/Vulkan/EVulkanShaderType.hpp"

namespace BetterThanNothing
{
	class CApplicationContext;
	struct CVulkanShaderModule;

	class CShaderPool
	{
	private:
		CApplicationContext* m_context;

		std::string m_basePath;
		std::unordered_map<std::string, CVulkanShaderModule> m_shaders;

	public:
		explicit CShaderPool(std::string  basePath, CApplicationContext* context): m_context(context), m_basePath(std::move(basePath)) {}
		~CShaderPool();

		CVulkanShaderModule LoadShader(const std::string& path, EVulkanShaderType type);
	private:
		static std::vector<char> _readFile(const std::string& filename);
		static glslang_stage_t _getShaderStage(EVulkanShaderType type);
		static glslang_program_t* _getShaderProgram(const std::vector<char>& source, glslang_stage_t stage);
		VkShaderModule _getShaderModule(glslang_program_t* program) const;
	};
};

