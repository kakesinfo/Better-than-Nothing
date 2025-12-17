#pragma once

#include "EVulkanShaderType.hpp"

namespace BetterThanNothing
{
	struct VertexInputDescription;
	struct VulkanContext;
	struct ShaderGpuData;

	struct VulkanPipelineShaderInfo
	{
		std::string shaderPath;
		EVulkanShaderType shaderType;
	};

	struct VulkanPipelineParams
	{
		const char* name;
		std::vector<VulkanPipelineShaderInfo> shadersInfos;
	};

	class CVulkanPipeline // TODO: Refactor pipelines to enable building Compute and RayTracing pipelines
	{
	public:
		explicit CVulkanPipeline(VulkanPipelineParams params, const VertexInputDescription& vertexInputDescription, const std::vector<const ShaderGpuData*>& shaders, VulkanContext* context);
		~CVulkanPipeline();

		[[nodiscard]]
		VkPipeline Handle() const;

		[[nodiscard]]
		VkPipelineLayout GetLayout() const;

	private:
		static VkShaderStageFlagBits _getShaderStageTypeByShaderType(EVulkanShaderType type);

		VulkanContext* m_context = nullptr;
		VulkanPipelineParams m_params;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
	};

	class CVulkanPipelineBuilder final
	{
	private:
		VulkanPipelineParams m_params{};

		bool m_hasVertexShader = false;
		bool m_hasFragmentShader = false;

	public:
		explicit CVulkanPipelineBuilder() = default;
		~CVulkanPipelineBuilder() = default;

		CVulkanPipelineBuilder& SetName(const char* name);
		CVulkanPipelineBuilder& AddShader(const std::string& shaderPath, EVulkanShaderType shaderType);
		CVulkanPipelineBuilder& AddVertexShader(const std::string& shaderPath);
		CVulkanPipelineBuilder& AddFragmentShader(const std::string& shaderPath);

		[[nodiscard]]
		VulkanPipelineParams& GetBuildParams();
	};
}; // BetterThanNothing
