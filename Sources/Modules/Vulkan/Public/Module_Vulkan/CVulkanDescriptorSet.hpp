#pragma once

namespace BetterThanNothing
{
	class CVulkanDescriptorSetLayout;
	struct VulkanContext;

	class CVulkanDescriptorSet
	{
	public:
		CVulkanDescriptorSet(const std::unique_ptr<CVulkanDescriptorSetLayout>& layout, VulkanContext* context);
		~CVulkanDescriptorSet();

		VkDescriptorSet Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
	};
}