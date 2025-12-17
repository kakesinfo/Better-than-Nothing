#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	CVulkanDescriptorSetLayout::CVulkanDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, VulkanContext* context): m_context(context)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<std::uint32_t>(layoutBindings.size());
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(context->vkDevice->Handle(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
			throw CVulkanDescriptorSetLayoutError("Failed to create descriptor set layout!");
	}

	CVulkanDescriptorSetLayout::~CVulkanDescriptorSetLayout()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_context->vkDevice->Handle(), m_descriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout CVulkanDescriptorSetLayout::Handle() const
	{
		return m_descriptorSetLayout;
	}
}