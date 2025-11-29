#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanImageView::CVulkanImageView(VkImageViewCreateInfo& createInfo, CVulkanContext* context): context(context)
	{
		if (vkCreateImageView(context->vkDevice->LogicalHandle(), &createInfo, nullptr, &m_imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
	}

	CVulkanImageView::~CVulkanImageView()
	{
		vkDestroyImageView(context->vkDevice->LogicalHandle(), m_imageView, nullptr);
	}
};