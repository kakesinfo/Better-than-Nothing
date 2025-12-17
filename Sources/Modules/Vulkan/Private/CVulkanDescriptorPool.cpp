#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	CVulkanDescriptorPool::CVulkanDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSizes, VulkanContext *context) : m_context(context)
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = s_calculateMaxSets(poolSizes); // 3 sets per frame (global + dynamic + sampler)

		if (vkCreateDescriptorPool(context->vkDevice->Handle(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool");
	}

	CVulkanDescriptorPool::~CVulkanDescriptorPool()
	{
		if (m_descriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_context->vkDevice->Handle(), m_descriptorPool, nullptr);
	}

	VkDescriptorPool CVulkanDescriptorPool::Handle() const
	{
		return m_descriptorPool;
	}

	std::uint32_t CVulkanDescriptorPool::s_calculateMaxSets(const std::vector<VkDescriptorPoolSize> &poolSizes)
	{
		std::uint32_t totalDescriptors = 0;
		for (const auto &[_, descriptorCount]: poolSizes)
			totalDescriptors += descriptorCount;
		return totalDescriptors;
	}
}
