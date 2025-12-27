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

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	VulkanDevice::VulkanDevice(VulkanContext* context) : m_context(context)
	{
		PickPhysicalDevice();
		GetPhysicalDeviceProperties();
		CreateLogicalDevice();
	}

	VulkanDevice::~VulkanDevice()
	{
		if (m_logicalDevice != VK_NULL_HANDLE)
			vkDestroyDevice(m_logicalDevice, nullptr);
	}

	void VulkanDevice::PickPhysicalDevice()
	{
		const auto vkInstance = m_context->vkInstance->Handle();

		std::uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

		m_physicalDevice = FindSuitableDevice(devices);

		Logger::Log<SUCCESS>("Vulkan physical device: ok");
	}

	void VulkanDevice::GetPhysicalDeviceProperties()
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

		m_vendorName = GetVendorById(deviceProperties.vendorID);
		m_deviceName = deviceProperties.deviceName;
		m_apiVersion = std::to_string(deviceProperties.apiVersion);
		m_minUniformBufferOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		m_maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;

		VkSampleCountFlags counts =
			deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_64_BIT;
		}
		else if (counts & VK_SAMPLE_COUNT_32_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_32_BIT;
		}
		else if (counts & VK_SAMPLE_COUNT_16_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_16_BIT;
		}
		else if (counts & VK_SAMPLE_COUNT_8_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_8_BIT;
		}
		else if (counts & VK_SAMPLE_COUNT_4_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
		}
		else if (counts & VK_SAMPLE_COUNT_2_BIT)
		{
			m_maxMsaaSamples = VK_SAMPLE_COUNT_2_BIT;
		}
		else
			m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
		if (!indices.IsComplete())
			throw std::runtime_error("Failed to find queue families");

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;

		for (std::uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.sampleRateShading = VK_TRUE; // Enable sample shading for better quality MSAA

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.pNext = &dynamicRenderingFeatures;

		const auto& deviceExtensions = m_context->vkSettings.deviceExtensions;
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (m_context->vkSettings.enableValidationLayers)
		{
			const auto& validationLayers = m_context->vkSettings.validationLayers;
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		Logger::Log<SUCCESS>("Vulkan logical device: ok");

		VkQueue graphicsQueue;
		vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
		m_context->vkGraphicsQueue = std::make_unique<VulkanQueue>(graphicsQueue);

		Logger::Log<SUCCESS>("Vulkan graphics queue: ok");

		VkQueue presentQueue;
		vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
		m_context->vkPresentQueue = std::make_unique<VulkanQueue>(presentQueue);

		Logger::Log<SUCCESS>("Vulkan presentation queue: ok");
	}

	VkFormat VulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
											   VkFormatFeatureFlags features) const
	{
		for (const VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::runtime_error("Failed to find supported format");
	}

	VkPhysicalDevice VulkanDevice::FindSuitableDevice(const std::vector<VkPhysicalDevice>& devices) const
	{
		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
				return device;
		}
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device) const noexcept
	{
		const QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);

		const bool queueFamiliesSupported = queueFamilyIndices.IsComplete();
		const bool extensionsSupported = CheckDeviceExtensionSupport(device);
		bool	   swapchainSupported = false;

		if (extensionsSupported)
		{
			auto swapChainSupport = QuerySwapChainSupport(device);
			swapchainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return queueFamiliesSupported && extensionsSupported && swapchainSupported &&
			   supportedFeatures.samplerAnisotropy;
	}

	bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) const noexcept
	{
		auto deviceExtensions = m_context->vkSettings.deviceExtensions;

		std::uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device) const
	{
		auto vkSurface = m_context->vkSurface->Handle();

		std::uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;

		std::uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// Check if the queue family supports graphics queue
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsFamily = i;

			// Check if the queue family supports present queue
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);
			if (presentSupport)
				indices.presentFamily = i;

			// If both graphics and present queues are supported, then we are done
			if (indices.IsComplete())
				break;
			i++;
		}

		return indices;
	}

	QueueFamilyIndices VulkanDevice::GetQueueFamilies() const
	{
		return FindQueueFamilies(m_physicalDevice);
	}

	void VulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(m_logicalDevice);
	}

	SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device) const
	{
		const auto surface = m_context->vkSurface->Handle();

		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		std::uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		std::uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	SwapChainSupportDetails VulkanDevice::GetSwapChainSupport() const
	{
		return QuerySwapChainSupport(m_physicalDevice);
	}

	std::string VulkanDevice::GetVendorById(const std::uint32_t vendorId) const
	{
		if (vendorId == 0x1002)
			return "AMD";
		if (vendorId == 0x1010)
			return "ImgTec";
		if (vendorId == 0x10DE)
			return "NVIDIA";
		if (vendorId == 0x13B5)
			return "ARM";
		if (vendorId == 0x5143)
			return "Qualcomm";
		if (vendorId == 0x8086)
			return "INTEL";
		return "Unknown";
	}

	VkPhysicalDevice VulkanDevice::PhysicalHandle() const
	{
		if (m_physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan physical device is not initialized");
		return m_physicalDevice;
	}

	VkDevice VulkanDevice::Handle() const
	{
		if (m_logicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan logical device is not initialized");
		return m_logicalDevice;
	}

	std::string VulkanDevice::GetVendorName() const
	{
		return m_vendorName;
	}

	std::string VulkanDevice::GetDeviceName() const
	{
		return m_deviceName;
	}

	std::string VulkanDevice::GetApiVersion() const
	{
		return m_apiVersion;
	}

	std::uint32_t VulkanDevice::GetMinUniformBufferOffsetAlignment() const
	{
		return m_minUniformBufferOffsetAlignment;
	}

	float VulkanDevice::GetMaxAnisotropy() const
	{
		return m_maxAnisotropy;
	}

	VkSampleCountFlagBits VulkanDevice::GetMaxMsaaSamples() const
	{
		return m_maxMsaaSamples;
	}
}
