#pragma once


namespace BetterThanNothing
{
	struct CVulkanContext;

	class CVulkanImageView
	{
	private:
		CVulkanContext* context = nullptr;
		VkImageView m_imageView = VK_NULL_HANDLE;

	public:
		explicit CVulkanImageView(VkImageViewCreateInfo& createInfo, CVulkanContext* context);
		~CVulkanImageView();

		CVulkanImageView(const CVulkanImageView&) = delete;
		CVulkanImageView& operator=(const CVulkanImageView&) = delete;
		CVulkanImageView(CVulkanImageView&& other) = delete;
		CVulkanImageView& operator=(CVulkanImageView&& other) = delete;

		[[nodiscard]]
		VkImageView Handle() const { return m_imageView; }
	};
};
