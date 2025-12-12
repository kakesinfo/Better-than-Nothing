#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;
	class CVulkanFramebuffer;
	class CVulkanImage;
	class CVulkanImageView;

	class CVulkanSwapChain {
	public:
		explicit CVulkanSwapChain(VulkanContext* context);
		~CVulkanSwapChain();

		CVulkanSwapChain(const CVulkanSwapChain&) = delete;
		CVulkanSwapChain& operator=(const CVulkanSwapChain&) = delete;
		CVulkanSwapChain(CVulkanSwapChain&& other) = delete;
		CVulkanSwapChain& operator=(CVulkanSwapChain&& other) = delete;

		void CreateImages();
		void CreateImageViews();
		void CreateFramebuffers();
		void CreateDepthResources();
		void CreateColorResources();

	private:
		void _createSwapchain();
		void _cleanupSwapchain();

	public:
		void RecreateSwapchain();

	private:
		[[nodiscard]]
		VkSurfaceFormatKHR _chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		[[nodiscard]]
		VkPresentModeKHR _choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		[[nodiscard]]
		VkExtent2D _chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		[[nodiscard]]
		VkFormat _chooseDepthFormat(const std::vector<VkFormat>& candidates);

	public:
		[[nodiscard]]
		VkFormat GetImageFormat() const;

		[[nodiscard]]
		uint32 GetImageCount() const;

		[[nodiscard]]
		VkExtent2D GetExtent() const;

		[[nodiscard]]
		uint32 GetWidth() const;

		[[nodiscard]]
		uint32 GetHeight() const;

		[[nodiscard]]
		float GetAspectRatio() const;

		[[nodiscard]]
		VkFramebuffer GetFramebuffer(uint32 index);

		[[nodiscard]]
		VkSwapchainKHR Handle() const;

		[[nodiscard]]
		VkFormat GetDepthFormat() const;

	private:
		VulkanContext* m_context = nullptr;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		VkFormat m_imageFormat;
		VkExtent2D m_extent;

		std::vector<VkImage> m_images;
		std::vector<std::unique_ptr<CVulkanImageView>> m_imageViews;
		std::vector<std::unique_ptr<CVulkanFramebuffer>> m_framebuffers;

		std::unique_ptr<CVulkanImage>     m_depthImage;
		std::unique_ptr<CVulkanImageView> m_depthImageView;
		VkFormat                          m_depthFormat;

		std::unique_ptr<CVulkanImage>     m_colorImage; // For multisampling
		std::unique_ptr<CVulkanImageView> m_colorImageView;
	};
};
