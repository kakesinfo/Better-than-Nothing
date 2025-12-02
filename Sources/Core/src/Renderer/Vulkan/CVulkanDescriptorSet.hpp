#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanDescriptorSetLayout;

	class CVulkanDescriptorSet
	{
	private:
		CVulkanContext* m_context = nullptr;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

	public:
		CVulkanDescriptorSet(const std::unique_ptr<CVulkanDescriptorSetLayout>& layout, CVulkanContext* context);
		~CVulkanDescriptorSet();

		VkDescriptorSet Handle() const;
	};
}