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
#include <set>

namespace BetterThanNothing
{
		Device::Device(const DeviceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		PickPhysicalDevice();
		GetPhysicalDeviceProperties();
		CreateLogicalDevice();
	}

	Device::~Device()
	{
		if (m_device != VK_NULL_HANDLE)
			vkDestroyDevice(m_device, nullptr);
	}

	std::unique_ptr<Device> Device::Create(const DeviceCreateInfo& createInfo)
	{
		return std::make_unique<Device>(createInfo);
	}

	void Device::PickPhysicalDevice()
	{
		std::uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_createInfo.instance->Handle(), &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_createInfo.instance->Handle(), &deviceCount, devices.data());

		m_physicalDevice = FindSuitableDevice(devices);

		Logger::Log<SUCCESS>("Vulkan physical device: ok");
	}

	void Device::GetPhysicalDeviceProperties()
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

		m_vendorName = GetVendorById(deviceProperties.vendorID);
		m_deviceName = deviceProperties.deviceName;
		m_apiVersion = std::to_string(deviceProperties.apiVersion);
		m_minUniformBufferOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		m_maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
		m_maxTexture2DSize = deviceProperties.limits.maxImageDimension2D;
		m_maxTexture3DSize = deviceProperties.limits.maxImageDimension3D;
		m_maxTextureCubeSize = deviceProperties.limits.maxImageDimensionCube;

		const VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;

		m_maxMsaaSamples = (counts > 0) ? static_cast<VkSampleCountFlagBits>(std::bit_floor(counts)) : VK_SAMPLE_COUNT_1_BIT;
	}

	void Device::CreateLogicalDevice()
	{
		if (m_device != VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan physical device already created");

		m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);
		if (!m_queueFamilyIndices.IsComplete())
			throw std::runtime_error("Failed to find queue families");

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set uniqueQueueFamilies = {m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value()};

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
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		const auto& deviceExtensions = m_createInfo.deviceExtensions;
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		Logger::Log<SUCCESS>("Vulkan logical device: ok");
	}

	std::unique_ptr<Queue> Device::CreateGraphicsQueue() const
	{
		const std::uint32_t familyIndex = m_queueFamilyIndices.graphicsFamily.value();

		VkQueue graphicsQueue;
		vkGetDeviceQueue(m_device, familyIndex, 0, &graphicsQueue);

		Logger::Log<SUCCESS>("Vulkan graphics queue: ok");

		return Queue::Create({graphicsQueue, familyIndex});
	}

	std::unique_ptr<Queue> Device::CreatePresentQueue() const
	{
		const std::uint32_t familyIndex = m_queueFamilyIndices.presentFamily.value();

		VkQueue presentQueue;
		vkGetDeviceQueue(m_device, familyIndex, 0, &presentQueue);

		Logger::Log<SUCCESS>("Vulkan present queue: ok");

		return Queue::Create({presentQueue, familyIndex});
	}

	VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
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

	bool Device::IsSupportedFormatFeature(VkFormat format, VkFormatFeatureFlags features) const
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		return props.optimalTilingFeatures & features;
	}

	void Device::WaitIdle() const
	{
		vkDeviceWaitIdle(m_device);
	}

	VkResult Device::CreateImageView(const VkImageViewCreateInfo& createInfo, VkImageView* imageView) const
	{
		return vkCreateImageView(m_device, &createInfo, nullptr, imageView);
	}

	void Device::DestroyImageView(VkImageView imageView) const
	{
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	VkResult Device::CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo, VkSampler* sampler) const
	{
		return vkCreateSampler(m_device, &samplerCreateInfo, nullptr, sampler);
	}

	void Device::DestroySampler(VkSampler sampler) const
	{
		vkDestroySampler(m_device, sampler, nullptr);
	}

	bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device) const noexcept
	{
		auto deviceExtensions = m_createInfo.deviceExtensions;

		std::uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& [extensionName, _] : availableExtensions)
		{
			requiredExtensions.erase(extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device) const
	{
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_createInfo.surface->Handle(), &presentSupport);
			if (presentSupport)
				indices.presentFamily = i;

			// If both graphics and present queues are supported, then we are done
			if (indices.IsComplete())
				break;
			i++;
		}

		return indices;
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device) const
	{
		const auto surface = m_createInfo.surface->Handle();

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

	bool Device::IsDeviceSuitable(VkPhysicalDevice device) const noexcept
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

		return queueFamiliesSupported && extensionsSupported && swapchainSupported && supportedFeatures.samplerAnisotropy;
	}

	VkPhysicalDevice Device::FindSuitableDevice(const std::vector<VkPhysicalDevice>& devices) const
	{
		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
				return device;
		}
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	std::string Device::GetVendorById(std::uint32_t vendorId)
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

	SwapChainSupportDetails Device::GetSwapChainSupport() const
	{
		return QuerySwapChainSupport(m_physicalDevice);
	}

	QueueFamilyIndices Device::GetQueueFamilyIndices() const
	{
		return m_queueFamilyIndices;
	}

	std::string Device::GetVendorName() const
	{
		return m_vendorName;
	}

	std::string Device::GetDeviceName() const
	{
		return m_deviceName;
	}

	std::string Device::GetApiVersion() const
	{
		return m_apiVersion;
	}

	std::uint32_t Device::GetMinUniformBufferOffsetAlignment() const
	{
		return m_minUniformBufferOffsetAlignment;
	}

	VkSampleCountFlagBits Device::GetMaxMsaaSamples() const
	{
		return m_maxMsaaSamples;
	}

	float Device::GetMaxAnisotropy() const
	{
		return m_maxAnisotropy;
	}

	std::uint32_t Device::GetMaxTexture2DSize() const
	{
		return m_maxTexture2DSize;
	}

	std::uint32_t Device::GetMaxTexture3DSize() const
	{
		return m_maxTexture3DSize;
	}

	std::uint32_t Device::GetMaxTextureCubeSize() const
	{
		return m_maxTextureCubeSize;
	}

	VkDevice Device::Handle() const
	{
		return m_device;
	}

	VkPhysicalDevice Device::PhysicalHandle() const
	{
		return m_physicalDevice;
	}
}
