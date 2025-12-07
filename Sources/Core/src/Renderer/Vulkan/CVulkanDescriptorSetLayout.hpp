//
// Created by kakesinfo on 01/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanDescriptorSetLayoutError final : public std::runtime_error
	{
	public:
		explicit CVulkanDescriptorSetLayoutError(const std::string& message)
			: std::runtime_error(message) {}
	};

	class CVulkanDescriptorSetLayout
	{
	public:
		explicit CVulkanDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, VulkanContext* context);
		~CVulkanDescriptorSetLayout();

		CVulkanDescriptorSetLayout(const CVulkanDescriptorSetLayout&) = delete;
		CVulkanDescriptorSetLayout& operator=(const CVulkanDescriptorSetLayout&) = delete;
		CVulkanDescriptorSetLayout(CVulkanDescriptorSetLayout&& other) = delete;
		CVulkanDescriptorSetLayout& operator=(CVulkanDescriptorSetLayout&& other) = delete;

		[[nodiscard]]
		VkDescriptorSetLayout Handle() const;

	private:
		VulkanContext* m_context;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	};
}