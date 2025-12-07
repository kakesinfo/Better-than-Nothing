//
// Created by acoezard on 11/6/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanDevice::CVulkanDevice(VulkanContext* context): m_context(context)
	{
		_pickPhysicalDevice();
		_getPhysicalDeviceProperties();
		_createLogicalDevice();

		LOG_INFO("Vendor: " << GetVendorName());
		LOG_INFO("Device: " << GetDeviceName());
		LOG_INFO("API Version: " << GetApiVersion());
	}

	CVulkanDevice::~CVulkanDevice()
	{
		if (m_logicalDevice != VK_NULL_HANDLE)
			vkDestroyDevice(m_logicalDevice, nullptr);
	}

	void CVulkanDevice::_pickPhysicalDevice()
	{
		auto vkInstance = m_context->vkInstance->Handle();

		uint32 deviceCount = 0;
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("Failed to find GPUs with Vulkan support");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

		m_physicalDevice = _findSuitableDevice(devices);

		LOG_SUCCESS("Vulkan physical device: ok");
	}

	void CVulkanDevice::_getPhysicalDeviceProperties()
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

		m_vendorName = _getVendorById(deviceProperties.vendorID);
		m_deviceName = deviceProperties.deviceName;
		m_apiVersion = std::to_string(deviceProperties.apiVersion);
	}

	void CVulkanDevice::_createLogicalDevice()
	{
		QueueFamilyIndices indices = _findQueueFamilies(m_physicalDevice);
		if (!indices.IsComplete())
			throw std::runtime_error("Failed to find queue families");

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set uniqueQueueFamilies = {
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		float32 queuePriority = 1.0f;

		for (uint32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		const auto& deviceExtensions = m_context->vkSettings.deviceExtensions;
		createInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (m_context->vkSettings.enableValidationLayers) {
			const auto& validationLayers = m_context->vkSettings.validationLayers;
			createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		LOG_SUCCESS("Vulkan logical device: ok");

		VkQueue graphicsQueue;
		vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
		m_context->vkGraphicsQueue = std::make_unique<CVulkanQueue>(graphicsQueue);

		LOG_SUCCESS("Vulkan graphics queue: ok");

		VkQueue presentQueue;
		vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
		m_context->vkPresentQueue = std::make_unique<CVulkanQueue>(presentQueue);

		LOG_SUCCESS("Vulkan presentation queue: ok");
	}

	VkPhysicalDevice CVulkanDevice::_findSuitableDevice(const std::vector<VkPhysicalDevice>& devices)
	{
		for (const auto& device : devices)
		{
			if (_isDeviceSuitable(device))
				return device;
		}
		throw std::runtime_error("Failed to find a suitable GPU");
	}

	bool CVulkanDevice::_isDeviceSuitable(VkPhysicalDevice device) noexcept
	{
		QueueFamilyIndices queueFamilyIndices = _findQueueFamilies(device);

		bool queueFamiliesSupported = queueFamilyIndices.IsComplete();
		bool extensionsSupported = _checkDeviceExtensionSupport(device);
		bool swapchainSupported = false;

		if (extensionsSupported)
		{
			auto swapChainSupport = _querySwapChainSupport(device);
			swapchainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return queueFamiliesSupported && extensionsSupported && swapchainSupported;
	}

	bool CVulkanDevice::_checkDeviceExtensionSupport(VkPhysicalDevice device) noexcept
	{
		auto deviceExtensions = m_context->vkSettings.deviceExtensions;

		uint32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices CVulkanDevice::_findQueueFamilies(VkPhysicalDevice device) const
	{
		auto vkSurface = m_context->vkSurface->Handle();

		uint32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;

		uint32 i = 0;
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

	QueueFamilyIndices CVulkanDevice::GetQueueFamilies() const
	{
		return _findQueueFamilies(m_physicalDevice);
	}

	void CVulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(m_logicalDevice);
	}

	SwapChainSupportDetails CVulkanDevice::_querySwapChainSupport(VkPhysicalDevice device) const
	{
		auto surface = m_context->vkSurface->Handle();

		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	SwapChainSupportDetails CVulkanDevice::GetSwapChainSupport() const
	{
		return _querySwapChainSupport(m_physicalDevice);
	}

	std::string CVulkanDevice::_getVendorById(const uint32 vendorId) const
	{
		if (vendorId == 0x1002) return "AMD";
		if (vendorId == 0x1010) return "ImgTec";
		if (vendorId == 0x10DE) return "NVIDIA";
		if (vendorId == 0x13B5) return "ARM";
		if (vendorId == 0x5143) return "Qualcomm";
		if (vendorId == 0x8086) return "INTEL";
		return "Unknown";
	}

	VkPhysicalDevice CVulkanDevice::PhysicalHandle() const
	{
		if (m_physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan physical device is not initialized");
		return m_physicalDevice;
	}

	VkDevice CVulkanDevice::Handle() const
	{
		if (m_logicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan logical device is not initialized");
		return m_logicalDevice;
	}

	std::string CVulkanDevice::GetVendorName() const
	{
		return m_vendorName;
	}

	std::string CVulkanDevice::GetDeviceName() const
	{
		return m_deviceName;
	}

	std::string CVulkanDevice::GetApiVersion() const
	{
		return m_apiVersion;
	}
}
