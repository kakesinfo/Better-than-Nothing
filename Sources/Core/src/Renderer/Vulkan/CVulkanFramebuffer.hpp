#pragma once

namespace BetterThanNothing
{
	struct CVulkanContext;

	struct CVulkanFramebuffer
	{
	private:
		CVulkanContext* m_context;
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

	public:
		explicit CVulkanFramebuffer(VkFramebufferCreateInfo* createInfo, CVulkanContext* context);
		~CVulkanFramebuffer();

		CVulkanFramebuffer(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer& operator=(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer(CVulkanFramebuffer&& other) = delete;
		CVulkanFramebuffer& operator=(CVulkanFramebuffer&& other) = delete;

		[[nodiscard]]
		VkFramebuffer Handle() const;
	};
}
