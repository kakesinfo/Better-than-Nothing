#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanImageView::CVulkanImageView(VkImageViewCreateInfo& createInfo, VulkanContext* context): context(context)
	{
		if (vkCreateImageView(context->vkDevice->Handle(), &createInfo, nullptr, &m_imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
	}

	CVulkanImageView::~CVulkanImageView()
	{
		vkDestroyImageView(context->vkDevice->Handle(), m_imageView, nullptr);
	}
};