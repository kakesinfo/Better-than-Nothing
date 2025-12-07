#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanFramebuffer
	{
	public:
		explicit CVulkanFramebuffer(VkFramebufferCreateInfo* createInfo, VulkanContext* context);
		~CVulkanFramebuffer();

		CVulkanFramebuffer(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer& operator=(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer(CVulkanFramebuffer&& other) = delete;
		CVulkanFramebuffer& operator=(CVulkanFramebuffer&& other) = delete;

		[[nodiscard]]
		VkFramebuffer Handle() const;

	private:
		VulkanContext* m_context;
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
	};
}
