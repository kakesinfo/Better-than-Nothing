/*
*	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
		std::uint32_t GetImageCount() const;

		[[nodiscard]]
		VkExtent2D GetExtent() const;

		[[nodiscard]]
		std::uint32_t GetWidth() const;

		[[nodiscard]]
		std::uint32_t GetHeight() const;

		[[nodiscard]]
		float GetAspectRatio() const;

		[[nodiscard]]
		VkFramebuffer GetFramebuffer(std::uint32_t index);

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
