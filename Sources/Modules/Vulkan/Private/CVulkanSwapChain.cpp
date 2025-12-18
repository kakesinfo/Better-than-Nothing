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

#include "module_vulkan.hpp"

#include <Module_Base/CLogger.hpp>
#include <Module_Platform/Manager/CWindowManager.hpp>
#include <Module_Platform/CWindow.hpp>

namespace BetterThanNothing
{
	CVulkanSwapChain::CVulkanSwapChain(VulkanContext* context): m_context(context)
	{
		_createSwapchain();
		CLogger::Log<Success>("Vulkan swap chain: ok");
	}

	CVulkanSwapChain::~CVulkanSwapChain()
	{
		_cleanupSwapchain();
	}

	void CVulkanSwapChain::_createSwapchain()
	{
		auto swapChainSupport = m_context->vkDevice->GetSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = _chooseSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = _choosePresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = _chooseExtent(swapChainSupport.capabilities);

		std::uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_context->vkSurface->Handle();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_context->vkDevice->GetQueueFamilies();
		std::uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // No transformation

		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No blending with other windows

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Clip obscured pixels

		createInfo.oldSwapchain = VK_NULL_HANDLE; // For recreation

		auto device = m_context->vkDevice->Handle();

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain!");

		m_imageFormat = surfaceFormat.format;
		m_extent = extent;
	}

	void CVulkanSwapChain::CreateImages()
	{
		auto device = m_context->vkDevice->Handle();

		std::uint32_t finalImageCount = 0;
		vkGetSwapchainImagesKHR(device, m_swapChain, &finalImageCount, nullptr);

		m_images.resize(finalImageCount);
		vkGetSwapchainImagesKHR(device, m_swapChain, &finalImageCount, m_images.data());
	}

	void CVulkanSwapChain::CreateImageViews()
	{
		const std::uint32_t imageCount = m_images.size();

		m_imageViews.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++)
			m_imageViews.at(i) = std::make_unique<CVulkanImageView>(m_context, m_images.at(i), m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void CVulkanSwapChain::CreateFramebuffers()
	{
		m_framebuffers.resize(m_imageViews.size());

		for (size_t i = 0; i < m_imageViews.size(); i++)
		{
			const auto colorImageView = m_colorImageView->Handle();
			const auto depthImageView = m_depthImageView->Handle();
			const auto imageView = m_imageViews.at(i)->Handle();
			const auto renderPass = m_context->vkRenderPass->Handle();

			const std::array attachments = { colorImageView, depthImageView, imageView};

			VkFramebufferCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.renderPass = renderPass;
			createInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
			createInfo.pAttachments = attachments.data();
			createInfo.width = m_extent.width;
			createInfo.height = m_extent.height;
			createInfo.layers = 1;

			auto framebuffer = std::make_unique<CVulkanFramebuffer>(&createInfo, m_context);
			m_framebuffers.at(i) = std::move(framebuffer);
		}
	}

	void CVulkanSwapChain::CreateDepthResources()
	{
		m_depthFormat = _chooseDepthFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT});

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_extent.width;
		imageInfo.extent.height = m_extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = m_context->vkDevice->GetMaxMsaaSamples();
		imageInfo.format = m_depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_depthImage = std::make_unique<CVulkanImage>(m_context, &imageInfo);
		m_depthImageView = std::make_unique<CVulkanImageView>(m_context, m_depthImage->Handle(), m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		CVulkanCommandBuffer::SingleTimeCommands([&](const std::unique_ptr<CVulkanCommandBuffer>& commandBuffer) {
			commandBuffer->TransitionImageLayout(m_depthImage->Handle(), m_depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
		}, m_context);
	}

	void CVulkanSwapChain::CreateColorResources()
	{
		VkFormat colorFormat = m_imageFormat;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_extent.width;
		imageInfo.extent.height = m_extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = m_context->vkDevice->GetMaxMsaaSamples();
		imageInfo.format = colorFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_colorImage = std::make_unique<CVulkanImage>(m_context, &imageInfo);
		m_colorImageView = std::make_unique<CVulkanImageView>(m_context, m_colorImage->Handle(), colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void CVulkanSwapChain::RecreateSwapchain()
	{
		auto& pWindow = CWindowManager::GetWindow();
		auto [width, height] = pWindow->GetFramebufferSize();

		while (width == 0 || height == 0)
		{
			auto framebufferSize = pWindow->GetFramebufferSize();
			width = framebufferSize.first;
			height = framebufferSize.second;

			glfwWaitEvents();
		}

		m_context->vkDevice->WaitIdle();

		_cleanupSwapchain();

		_createSwapchain();
		CreateImages();
		CreateImageViews();
		CreateColorResources();
		CreateDepthResources();
		CreateFramebuffers();
	}

	void CVulkanSwapChain::_cleanupSwapchain()
	{
		m_framebuffers.clear();
		m_depthImageView.reset();
		m_depthImage.reset();
		m_colorImageView.reset();
		m_colorImage.reset();
		m_imageViews.clear();

		vkDestroySwapchainKHR(m_context->vkDevice->Handle(), m_swapChain, nullptr);
	}

	VkSurfaceFormatKHR CVulkanSwapChain::_chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& surfaceFormat : availableFormats)
		{
			const bool correctFormat = surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB;
			const bool correctColorSpace = surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			if (correctFormat && correctColorSpace)
				return surfaceFormat;
		}
		return availableFormats[0];
	}

	VkPresentModeKHR CVulkanSwapChain::_choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // TripleBuffering buffering
				return availablePresentMode;
		}
		return VK_PRESENT_MODE_FIFO_KHR; // V-Sync
	}

	VkExtent2D CVulkanSwapChain::_chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
			return capabilities.currentExtent;

		auto [width, height] = CWindowManager::GetWindow()->GetFramebufferSize();

		VkExtent2D actualExtent;
		actualExtent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	VkFormat CVulkanSwapChain::_chooseDepthFormat(const std::vector<VkFormat>& candidates)
	{
		return  m_context->vkDevice->FindSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat CVulkanSwapChain::GetImageFormat() const
	{
		return m_imageFormat;
	}

	std::uint32_t CVulkanSwapChain::GetImageCount() const
	{
		return m_images.size();
	}

	VkExtent2D CVulkanSwapChain::GetExtent() const
	{
		return m_extent;
	}

	std::uint32_t CVulkanSwapChain::GetWidth() const
	{
		return m_extent.width;
	}

	std::uint32_t CVulkanSwapChain::GetHeight() const
	{
		return m_extent.height;
	}

	float CVulkanSwapChain::GetAspectRatio() const
	{
		return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
	}

	VkFramebuffer CVulkanSwapChain::GetFramebuffer(std::uint32_t imageIndex)
	{
		if (imageIndex >= m_framebuffers.size())
			throw std::runtime_error("Framebuffer index out of range");
		return m_framebuffers.at(imageIndex)->Handle();
	}

	VkSwapchainKHR CVulkanSwapChain::Handle() const
	{
		return m_swapChain;
	}

	VkFormat CVulkanSwapChain::GetDepthFormat() const
	{
		return m_depthFormat;
	}
};
