#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanFramebuffer::CVulkanFramebuffer(VkFramebufferCreateInfo* createInfo, VulkanContext* context)
		: m_context(context)
	{
		auto device = context->vkDevice->Handle();

		if (vkCreateFramebuffer(device, createInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create framebuffer!");
	}

	CVulkanFramebuffer::~CVulkanFramebuffer()
	{
		vkDestroyFramebuffer(m_context->vkDevice->Handle(), m_framebuffer, nullptr);
	}

	VkFramebuffer CVulkanFramebuffer::Handle() const
	{
		return m_framebuffer;
	}
}