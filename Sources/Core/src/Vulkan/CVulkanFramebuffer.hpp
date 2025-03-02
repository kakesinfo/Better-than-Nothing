#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanFramebuffer
	{
	private:
		CApplicationContext* m_context;
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

	public:
		explicit CVulkanFramebuffer(VkFramebufferCreateInfo* createInfo, CApplicationContext* context);
		~CVulkanFramebuffer();

		CVulkanFramebuffer(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer& operator=(const CVulkanFramebuffer&) = delete;
		CVulkanFramebuffer(CVulkanFramebuffer&& other) = delete;
		CVulkanFramebuffer& operator=(CVulkanFramebuffer&& other) = delete;

		[[nodiscard]]
		VkFramebuffer Handle() const;
	};
}
