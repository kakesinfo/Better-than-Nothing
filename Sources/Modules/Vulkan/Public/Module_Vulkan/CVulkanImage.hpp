#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanImage
	{
	public:
		CVulkanImage(VulkanContext* context, const VkImageCreateInfo* imageCreateInfo);
		~CVulkanImage();

		CVulkanImage(const CVulkanImage&) = delete;
		CVulkanImage& operator=(const CVulkanImage&) = delete;

		[[nodiscard]]
		VkImage Handle() const;

	private:
		VulkanContext* m_context;

		VkImage m_image = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_allocationInfo{};
	};
}

