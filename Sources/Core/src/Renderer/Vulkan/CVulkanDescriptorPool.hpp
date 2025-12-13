#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CVulkanDescriptorPool
	{
	public:
		explicit CVulkanDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSizes, VulkanContext *context);

		~CVulkanDescriptorPool();

		VkDescriptorPool Handle() const;

	private:
		static uint32 s_calculateMaxSets(const std::vector<VkDescriptorPoolSize> &poolSizes);

		VulkanContext *m_context = nullptr;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	};
} // BetterThanNothing
