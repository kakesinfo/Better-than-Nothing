//
// Created by acoezard on 11/19/24.
//

#pragma once

#include "EVulkanShaderType.hpp"
#include "CVulkanShaderModule.hpp"

namespace BetterThanNothing
{
	class CApplicationContext;

	struct VulkanPipelineParams
	{
		const char* name;
		std::vector<CVulkanShaderModule> shadersInfos;
	};

	class CVulkanPipeline // TODO: Refactor pipelines to enable building Compute and RayTracing pipelines
	{
	private:
		CApplicationContext* m_context = nullptr;
		VulkanPipelineParams m_params;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;

	public:
		explicit CVulkanPipeline(VulkanPipelineParams  params, CApplicationContext* context);
		~CVulkanPipeline();

		[[nodiscard]]
		VkPipeline Handle() const;

	private:
		static VkShaderStageFlagBits _getShaderStageTypeByShaderType(EVulkanShaderType type);
	};

	class VulkanPipelineBuilder
	{
	private:
		VulkanPipelineParams m_params{};

		bool m_hasVertexShader = false;
		bool m_hasFragmentShader = false;

	public:
		explicit VulkanPipelineBuilder() = default;
		~VulkanPipelineBuilder() = default;

		VulkanPipelineBuilder& SetName(const char* name);
		VulkanPipelineBuilder& AddShader(const std::string& shaderPath, EVulkanShaderType shaderType);
		VulkanPipelineBuilder& AddVertexShader(const std::string& shaderPath);
		VulkanPipelineBuilder& AddFragmentShader(const std::string& shaderPath);

		[[nodiscard]]
		VulkanPipelineParams& GetBuildParams();
	};
}; // BetterThanNothing
