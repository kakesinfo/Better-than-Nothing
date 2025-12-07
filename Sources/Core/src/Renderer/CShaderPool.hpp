//
// Created by acoezard on 11/19/24.
//

#pragma once

#include "Renderer/Vulkan/EVulkanShaderType.hpp"
#include "Renderer/Vulkan/VulkanShaderModule.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;
	struct VulkanShaderModule;

	class CShaderPool
	{
	public:
		explicit CShaderPool(std::string basePath, VulkanContext* context);
		~CShaderPool();

		const VulkanShaderModule& LoadShader(const std::string& path, EVulkanShaderType type);

	private:
		static std::vector<char> _readFile(const std::string& filename);
		static glslang_stage_t _getShaderStage(EVulkanShaderType type);
		static glslang_program_t* _getShaderProgram(const std::vector<char>& source, glslang_stage_t stage);
		VkShaderModule _getShaderModule(glslang_program_t* program) const;

		VulkanContext* m_context;
		std::string m_basePath;
		std::unordered_map<std::string, VulkanShaderModule> m_shaders;
	};
};

