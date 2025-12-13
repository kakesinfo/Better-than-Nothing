//
// Created by acoezard on 11/19/24.
//

#pragma once

#include "EVulkanShaderType.hpp"
#include "VulkanShaderModule.hpp"
#include "Public/IPipelineBuilder.hpp"

namespace BetterThanNothing
{
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
		explicit CVulkanPipeline(VulkanPipelineParams params, std::vector<const ShaderGpuData*> shaders, VulkanContext* context);
		~CVulkanPipeline();

		[[nodiscard]]
		VkPipeline Handle() const;

		[[nodiscard]]
		VkPipelineLayout GetLayout() const;

	private:
		static VkShaderStageFlagBits _getShaderStageTypeByShaderType(EVulkanShaderType type);
		static VkVertexInputBindingDescription _getVertexBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 4> _getVertexAttributeDescriptions();

		VulkanContext* m_context = nullptr;
		VulkanPipelineParams m_params;

		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
	};

	class VulkanPipelineBuilder final : public IPipelineBuilder
	{
	private:
		VulkanPipelineParams m_params{};

		bool m_hasVertexShader = false;
		bool m_hasFragmentShader = false;

	public:
		explicit VulkanPipelineBuilder() = default;
		~VulkanPipelineBuilder() override = default;

		VulkanPipelineBuilder& SetName(const char* name) override;
		VulkanPipelineBuilder& AddShader(const std::string& shaderPath, EVulkanShaderType shaderType);
		VulkanPipelineBuilder& AddVertexShader(const std::string& shaderPath) override;
		VulkanPipelineBuilder& AddFragmentShader(const std::string& shaderPath) override;

		[[nodiscard]]
		VulkanPipelineParams& GetBuildParams();
	};
}; // BetterThanNothing
