#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanDescriptorPool
	{
	public:
		explicit CVulkanDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSizes, VulkanContext *context);

		~CVulkanDescriptorPool();

		VkDescriptorPool Handle() const;

	private:
		static std::uint32_t s_calculateMaxSets(const std::vector<VkDescriptorPoolSize> &poolSizes);

		VulkanContext *m_context = nullptr;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	};
} // BetterThanNothing
