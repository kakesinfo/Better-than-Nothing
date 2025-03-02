#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanImageView::CVulkanImageView(VkImageViewCreateInfo& createInfo, CApplicationContext* context): context(context)
	{
		if (vkCreateImageView(context->GetVulkanDevice()->LogicalHandle(), &createInfo, nullptr, &m_imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view");
	}

	CVulkanImageView::~CVulkanImageView()
	{
		vkDestroyImageView(context->GetVulkanDevice()->LogicalHandle(), m_imageView, nullptr);
	}
};