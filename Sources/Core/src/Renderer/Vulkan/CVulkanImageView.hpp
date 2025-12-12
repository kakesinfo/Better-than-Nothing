#pragma once


namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanImageView
	{
	public:
		explicit CVulkanImageView(VulkanContext* context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32 mipLevels);
		~CVulkanImageView();

		CVulkanImageView(const CVulkanImageView&) = delete;
		CVulkanImageView& operator=(const CVulkanImageView&) = delete;
		CVulkanImageView(CVulkanImageView&& other) = delete;
		CVulkanImageView& operator=(CVulkanImageView&& other) = delete;

		[[nodiscard]]
		VkImageView Handle() const { return m_imageView; }

	private:
		VulkanContext* context = nullptr;
		VkImageView m_imageView = VK_NULL_HANDLE;
	};
};
