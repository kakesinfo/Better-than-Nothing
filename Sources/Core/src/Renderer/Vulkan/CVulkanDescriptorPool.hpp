#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanDescriptorPool
	{
	private:
		CVulkanContext* m_context = nullptr;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

	public:
		explicit CVulkanDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, CVulkanContext* context);
		~CVulkanDescriptorPool();

		VkDescriptorPool Handle() const;
	};
} // BetterThanNothing