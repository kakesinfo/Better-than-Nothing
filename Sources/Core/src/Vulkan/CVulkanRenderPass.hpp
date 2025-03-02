#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanRenderPass
	{
	private:
		CApplicationContext* m_context = nullptr;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

	public:
		explicit CVulkanRenderPass(CApplicationContext* context);
		~CVulkanRenderPass();

		CVulkanRenderPass(const CVulkanRenderPass&) = delete;
		CVulkanRenderPass& operator=(const CVulkanRenderPass&) = delete;
		CVulkanRenderPass(CVulkanRenderPass&& other) = delete;
		CVulkanRenderPass& operator=(CVulkanRenderPass&& other) = delete;

		[[nodiscard]]
		VkRenderPass Handle() const;
	};
}