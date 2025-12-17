#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanRenderPass
	{
	public:
		explicit CVulkanRenderPass(VulkanContext* context);
		~CVulkanRenderPass();

		CVulkanRenderPass(const CVulkanRenderPass&) = delete;
		CVulkanRenderPass& operator=(const CVulkanRenderPass&) = delete;
		CVulkanRenderPass(CVulkanRenderPass&& other) = delete;
		CVulkanRenderPass& operator=(CVulkanRenderPass&& other) = delete;

		[[nodiscard]]
		VkRenderPass Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
	};
}