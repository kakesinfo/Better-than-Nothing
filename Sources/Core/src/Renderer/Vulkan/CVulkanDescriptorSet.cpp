#include "CVulkanDescriptorSet.hpp"

namespace BetterThanNothing
{
	CVulkanDescriptorSet::CVulkanDescriptorSet(const std::unique_ptr<CVulkanDescriptorSetLayout>& layout, VulkanContext* context)
		: m_context(context)
	{
		auto handle = layout->Handle();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_context->vkDescriptorPool->Handle();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &handle;

		if (vkAllocateDescriptorSets(m_context->vkDevice->Handle(), &allocInfo, &m_descriptorSet) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor set");
	}

	CVulkanDescriptorSet::~CVulkanDescriptorSet()
	{
		// Note: Descriptor sets are automatically freed when the descriptor pool is destroyed.
	}

	VkDescriptorSet CVulkanDescriptorSet::Handle() const
	{
		return m_descriptorSet;
	}
} // BetterThanNothing