/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "RenderingApi.hpp"
#include <Common/Logger.hpp>

namespace BetterThanNothing::RenderingApi
{
	SwapChain::SwapChain(const SwapChainCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		CreateSwapChain(createInfo.width, createInfo.height);
		CreateImages();
		CreateSyncObjects();

		Logger::Log<SUCCESS>("Vulkan swap chain: ok");
	}

	SwapChain::~SwapChain()
	{
		CleanupSwapChain();
	}

	std::unique_ptr<SwapChain> SwapChain::Create(const SwapChainCreateInfo& createInfo)
	{
		return std::make_unique<SwapChain>(createInfo);
	}

	void SwapChain::CreateSwapChain(const uint32_t width, const uint32_t height)
	{
		auto swapChainSupport = m_createInfo.device->GetSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR   presentMode = ChoosePresentMode(swapChainSupport.presentModes, m_createInfo.bufferingMode);
		VkExtent2D		   extent = ChooseExtent(swapChainSupport.capabilities, width, height);
		uint32_t		   imageCount = ChooseImageCount(swapChainSupport.capabilities, presentMode);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_createInfo.surface->Handle();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_createInfo.device->GetQueueFamilyIndices();
		uint32_t		   queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // No transformation
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;			  // No blending with other windows
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;			  // Clip obscured pixels
		createInfo.oldSwapchain = VK_NULL_HANDLE; // For recreation

		auto device = m_createInfo.device->Handle();

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain!");

		m_imageFormat = surfaceFormat.format;
		m_extent = extent;
	}

	void SwapChain::CreateImages()
	{
		auto device = m_createInfo.device->Handle();

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr);

		m_images.resize(imageCount);
		m_imageViews.resize(imageCount);

		vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, m_images.data());

		for (size_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_imageFormat;
			createInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			createInfo.image = m_images.at(i);

			if (vkCreateImageView(device, &createInfo, nullptr, &m_imageViews.at(i)) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image view");
		}
	}

	void SwapChain::CreateSyncObjects()
	{
		const auto imageCount = static_cast<uint32_t>(m_images.size());

		m_imageFinishedSemaphores.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++)
			m_imageFinishedSemaphores[i] = Semaphore::Create({m_createInfo.device});
	}

	std::optional<uint32_t> SwapChain::AcquireNextImage(const VkSemaphore imageAvailableSemaphore) const
	{
		uint32_t	   imageIndex;
		const VkResult result = vkAcquireNextImageKHR(m_createInfo.device->Handle(), m_swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			return std::nullopt;

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swap chain image!");

		return imageIndex;
	}

	void SwapChain::RecreateSwapChain(const uint32_t newWidth, const uint32_t newHeight)
	{
		if (newWidth == 0 || newHeight == 0)
			return;

		m_createInfo.device->WaitIdle();

		CleanupSwapChain();

		CreateSwapChain(newWidth, newHeight);
		CreateImages();
		CreateSyncObjects();
		++m_generation;
	}

	void SwapChain::CleanupSwapChain()
	{
		m_imageFinishedSemaphores.clear();

		for (const VkImageView view : m_imageViews)
			m_createInfo.device->DestroyImageView(view);

		m_imageViews.clear();
		m_images.clear();

		if (m_swapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_createInfo.device->Handle(), m_swapChain, nullptr);
			m_swapChain = VK_NULL_HANDLE;
		}
	}

	VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& surfaceFormat : availableFormats)
		{
			const bool correctFormat = surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM; // disable Gamma Correction
			const bool correctColorSpace = surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			if (correctFormat && correctColorSpace)
				return surfaceFormat;
		}
		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const EBufferingMode bufferingMode)
	{
		VkPresentModeKHR targetMode;

		switch (bufferingMode)
		{
			case EBufferingMode::Triple:
				targetMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			case EBufferingMode::Immediate:
				targetMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				break;
			case EBufferingMode::Double:
			default:
				return VK_PRESENT_MODE_FIFO_KHR;
		}

		for (const auto& availableMode : availablePresentModes)
		{
			if (availableMode == targetMode)
				return targetMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;

		VkExtent2D actualExtent;
		actualExtent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	uint32_t SwapChain::ChooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities, const VkPresentModeKHR& presentMode)
	{
		uint32_t imageCount = capabilities.minImageCount + 1;

		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR && imageCount < 3)
			imageCount = 3;

		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		return imageCount;
	}

	VkExtent2D SwapChain::GetExtent() const
	{
		return m_extent;
	}

	uint32_t SwapChain::GetWidth() const
	{
		return m_extent.width;
	}

	uint32_t SwapChain::GetHeight() const
	{
		return m_extent.height;
	}

	float SwapChain::GetAspectRatio() const
	{
		if (m_extent.height == 0)
			return 1.0f;
		return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
	}

	uint32_t SwapChain::GetImageCount() const
	{
		return static_cast<uint32_t>(m_images.size());
	}

	VkFormat SwapChain::GetImageFormat() const
	{
		return m_imageFormat;
	}

	VkImage SwapChain::GetImage(const uint32_t index) const
	{
		if (index >= m_images.size())
			throw std::runtime_error("Image index out of range");
		return m_images.at(index);
	}

	VkImageView SwapChain::GetImageView(const uint32_t index) const
	{
		if (index >= m_imageViews.size())
			throw std::runtime_error("Image view index out of range");
		return m_imageViews.at(index);
	}

	VkSemaphore SwapChain::GetImageFinishedSemaphore(const uint32_t index) const
	{
		if (index >= m_imageFinishedSemaphores.size())
			throw std::runtime_error("Image finished semaphore index out of range");
		return m_imageFinishedSemaphores.at(index)->Handle();
	}

	uint64_t SwapChain::GetGeneration() const
	{
		return m_generation;
	}

	VkSwapchainKHR SwapChain::Handle() const
	{
		return m_swapChain;
	}
};
