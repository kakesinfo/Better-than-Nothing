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

#include "RenderingApi.hpp"
#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	SwapChain::SwapChain(const SwapChainCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		CreateSwapChain(createInfo.width, createInfo.height);
		CreateImages();
		CreateColorResources();
		CreateDepthResources();

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

		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No blending with other windows

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Clip obscured pixels

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

	void SwapChain::CreateDepthResources()
	{
		m_depthFormat = ChooseDepthFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT});

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_extent.width;
		imageInfo.extent.height = m_extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = m_createInfo.device->GetMaxMsaaSamples();
		imageInfo.format = m_depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		if (m_createInfo.allocator->CreateImage(&imageInfo, &allocCreateInfo, &m_depthImage, &m_depthAllocation, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Cannot allocate depth image");

		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || m_depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_depthImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_depthFormat;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (m_createInfo.device->CreateImageView(createInfo, &m_depthImageView) != VK_SUCCESS)
			throw std::runtime_error("Cannot allocate depth image view");

		CommandBuffer::SingleTimeCommands(
			[&](const std::unique_ptr<CommandBuffer>& commandBuffer) {
				commandBuffer->TransitionImageLayout(m_depthImage, m_depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
			},
			m_createInfo.device, m_createInfo.commandPool, m_createInfo.graphicsQueue
		);
	}

	void SwapChain::CreateColorResources()
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_extent.width;
		imageInfo.extent.height = m_extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = m_createInfo.device->GetMaxMsaaSamples();
		imageInfo.format = m_imageFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		if (m_createInfo.allocator->CreateImage(&imageInfo, &allocCreateInfo, &m_colorImage, &m_colorAllocation, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Cannot allocate color image");

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_colorImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_imageFormat;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (m_createInfo.device->CreateImageView(createInfo, &m_colorImageView) != VK_SUCCESS)
			throw std::runtime_error("Cannot allocate depth image view");
	}

	void SwapChain::RecreateSwapChain(const uint32_t newWidth, const uint32_t newHeight)
	{
		m_createInfo.device->WaitIdle();

		CleanupSwapChain();

		CreateSwapChain(newWidth, newHeight);
		CreateImages();
		CreateColorResources();
		CreateDepthResources();
	}

	void SwapChain::CleanupSwapChain()
	{
		m_createInfo.device->DestroyImageView(m_depthImageView);
		m_createInfo.allocator->DestroyImage(m_depthImage, m_depthAllocation);

		m_createInfo.device->DestroyImageView(m_colorImageView);
		m_createInfo.allocator->DestroyImage(m_colorImage, m_colorAllocation);

		for (size_t i = 0; i < m_images.size(); i++)
			m_createInfo.device->DestroyImageView(m_imageViews.at(i));

		m_imageViews.clear();
		m_images.clear();

		vkDestroySwapchainKHR(m_createInfo.device->Handle(), m_swapChain, nullptr);
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

	VkFormat SwapChain::ChooseDepthFormat(const std::vector<VkFormat>& candidates) const
	{
		return m_createInfo.device->FindSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
		return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
	}

	uint32_t SwapChain::GetImageCount() const
	{
		return m_images.size();
	}

	VkFormat SwapChain::GetImageFormat() const
	{
		return m_imageFormat;
	}

	VkImage SwapChain::GetImage(uint32_t index) const
	{
		if (index >= m_images.size())
			throw std::runtime_error("Image index out of range");
		return m_images.at(index);
	}

	VkImageView SwapChain::GetImageView(uint32_t index) const
	{
		if (index >= m_imageViews.size())
			throw std::runtime_error("Image view index out of range");
		return m_imageViews.at(index);
	}

	VkFormat SwapChain::GetDepthFormat() const
	{
		return m_depthFormat;
	}

	VkImage SwapChain::GetDepthImage() const
	{
		if (!m_depthImage)
			throw std::runtime_error("Depth image not created");
		return m_depthImage;
	}

	VkImageView SwapChain::GetDepthImageView() const
	{
		if (!m_depthImageView)
			throw std::runtime_error("Depth image view not created");
		return m_depthImageView;
	}

	VkFormat SwapChain::GetColorFormat() const
	{
		return m_imageFormat;
	}

	VkImage SwapChain::GetColorImage() const
	{
		if (!m_colorImage)
			throw std::runtime_error("Color image not created");
		return m_colorImage;
	}

	VkImageView SwapChain::GetColorImageView() const
	{
		if (!m_colorImageView)
			throw std::runtime_error("Color image view not created");
		return m_colorImageView;
	}

	VkSwapchainKHR SwapChain::Handle() const
	{
		return m_swapChain;
	}
};
