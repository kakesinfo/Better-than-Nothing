#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanSampler
	{
	public:
		explicit CVulkanSampler(VulkanContext* context, const VkSamplerCreateInfo* samplerCreateInfo);
		~CVulkanSampler();

		CVulkanSampler(const CVulkanSampler&) = delete;
		CVulkanSampler& operator=(const CVulkanSampler&) = delete;

		[[nodiscard]]
		VkSampler Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}

