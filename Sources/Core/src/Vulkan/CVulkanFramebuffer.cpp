#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanFramebuffer::CVulkanFramebuffer(VkFramebufferCreateInfo* createInfo, CApplicationContext* context)
		: m_context(context)
	{
		auto device = context->GetVulkanDevice()->LogicalHandle();

		if (vkCreateFramebuffer(device, createInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create framebuffer!");
	}

	CVulkanFramebuffer::~CVulkanFramebuffer()
	{
		vkDestroyFramebuffer(m_context->GetVulkanDevice()->LogicalHandle(), m_framebuffer, nullptr);
	}

	VkFramebuffer CVulkanFramebuffer::Handle() const
	{
		return m_framebuffer;
	}
}