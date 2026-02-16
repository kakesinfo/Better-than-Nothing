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
	class Instance;
	class Surface;
	class Device;

	struct SwapChainCreateInfo
	{
		Surface*	   surface;
		Device*		   device;
		Allocator*	   allocator;
		CommandPool*   commandPool;
		Queue*		   graphicsQueue;
		uint32_t	   width;
		uint32_t	   height;
		EBufferingMode bufferingMode = EBufferingMode::Double;
	};

	class SwapChain
	{
	public:
		explicit SwapChain(const SwapChainCreateInfo& createInfo);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&& other) = delete;
		SwapChain& operator=(SwapChain&& other) = delete;

		[[nodiscard]] static std::unique_ptr<SwapChain> Create(const SwapChainCreateInfo& createInfo);

	private:
		void CreateSwapChain(uint32_t width, uint32_t height);
		void CreateImages();
		void CleanupSwapChain();

		[[nodiscard]] static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		[[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, EBufferingMode bufferingMode);
		[[nodiscard]] static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);
		[[nodiscard]] static uint32_t	ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities, const VkPresentModeKHR& presentMode);
		[[nodiscard]] VkFormat			ChooseDepthFormat(const std::vector<VkFormat>& candidates) const;

	public:
		void CreateDepthResources();
		void CreateColorResources();

		void RecreateSwapChain(uint32_t newWidth, uint32_t newHeight);

		[[nodiscard]] VkExtent2D	 GetExtent() const;
		[[nodiscard]] uint32_t		 GetWidth() const;
		[[nodiscard]] uint32_t		 GetHeight() const;
		[[nodiscard]] float			 GetAspectRatio() const;
		[[nodiscard]] uint32_t		 GetImageCount() const;
		[[nodiscard]] VkFormat		 GetImageFormat() const;
		[[nodiscard]] VkImage		 GetImage(uint32_t index) const;
		[[nodiscard]] VkImageView	 GetImageView(uint32_t index) const;
		[[nodiscard]] VkFormat		 GetDepthFormat() const;
		[[nodiscard]] VkImage		 GetDepthImage() const;
		[[nodiscard]] VkImageView	 GetDepthImageView() const;
		[[nodiscard]] VkFormat		 GetColorFormat() const;
		[[nodiscard]] VkImage		 GetColorImage() const;
		[[nodiscard]] VkImageView	 GetColorImageView() const;
		[[nodiscard]] VkSwapchainKHR Handle() const;

	private:
		const SwapChainCreateInfo m_createInfo;
		VkSwapchainKHR			  m_swapChain = VK_NULL_HANDLE;
		VkExtent2D				  m_extent{};
		VkFormat				  m_imageFormat = VK_FORMAT_UNDEFINED;
		std::vector<VkImage>	  m_images;
		std::vector<VkImageView>  m_imageViews;
		VkImage					  m_depthImage = VK_NULL_HANDLE;
		VkImageView				  m_depthImageView = VK_NULL_HANDLE;
		VkFormat				  m_depthFormat = VK_FORMAT_UNDEFINED;
		VmaAllocation			  m_depthAllocation = VK_NULL_HANDLE;
		VkImage					  m_colorImage = VK_NULL_HANDLE; // For multisampling
		VkImageView				  m_colorImageView = VK_NULL_HANDLE;
		VmaAllocation			  m_colorAllocation = VK_NULL_HANDLE;
	};
};
