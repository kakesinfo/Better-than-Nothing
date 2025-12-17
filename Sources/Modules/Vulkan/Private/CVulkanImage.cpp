#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	CVulkanImage::CVulkanImage(VulkanContext* context, const VkImageCreateInfo* imageCreateInfo) : m_context(context)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		if (m_context->vkMemoryAllocator->CreateImage(imageCreateInfo, &allocCreateInfo, &m_image, &m_allocation, &m_allocationInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan image.");
	}

	CVulkanImage::~CVulkanImage()
	{
		if (m_image && m_allocation)
			m_context->vkMemoryAllocator->DestroyImage(m_image, m_allocation);
	}

	VkImage CVulkanImage::Handle() const
	{
		return m_image;
	}
}