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
#include <bit>
#include <cstring>
#include <set>

namespace BetterThanNothing::RenderingApi
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
		uint32_t deviceCount = 0;
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
		m_apiVersion = std::format("{}.{}.{}", VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion));
		m_minUniformBufferOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		m_maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
		m_maxTexture2DSize = deviceProperties.limits.maxImageDimension2D;
		m_maxTexture3DSize = deviceProperties.limits.maxImageDimension3D;
		m_maxTextureCubeSize = deviceProperties.limits.maxImageDimensionCube;

		const VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts & deviceProperties.limits.framebufferDepthSampleCounts;

		m_maxMsaaSamples = (counts > 0) ? static_cast<VkSampleCountFlagBits>(std::bit_floor(counts)) : VK_SAMPLE_COUNT_1_BIT;

		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		if (HasRequiredRayQueryExtensions(availableExtensions))
		{
			VkPhysicalDeviceRayQueryFeaturesKHR rqFeatures{};
			rqFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

			VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures{};
			asFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			asFeatures.pNext = &rqFeatures;

			VkPhysicalDeviceBufferDeviceAddressFeatures bdaFeatures{};
			bdaFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
			bdaFeatures.pNext = &asFeatures;

			VkPhysicalDeviceFeatures2 features2{};
			features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features2.pNext = &bdaFeatures;
			vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features2);

			if (bdaFeatures.bufferDeviceAddress && asFeatures.accelerationStructure && rqFeatures.rayQuery)
			{
				m_rayQuerySupported = true;

				VkPhysicalDeviceAccelerationStructurePropertiesKHR asProps{};
				asProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

				VkPhysicalDeviceProperties2 props2{};
				props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
				props2.pNext = &asProps;
				vkGetPhysicalDeviceProperties2(m_physicalDevice, &props2);

				if (asProps.minAccelerationStructureScratchOffsetAlignment > 0)
					m_minAccelerationStructureScratchOffsetAlignment = asProps.minAccelerationStructureScratchOffsetAlignment;
			}
		}
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
		for (uint32_t queueFamily : uniqueQueueFamilies)
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

		VkPhysicalDeviceSynchronization2Features sync2Features{};
		sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		sync2Features.synchronization2 = VK_TRUE;

		VkPhysicalDeviceBufferDeviceAddressFeatures bdaFeatures{};
		bdaFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		bdaFeatures.bufferDeviceAddress = VK_TRUE;

		VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures{};
		asFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		asFeatures.accelerationStructure = VK_TRUE;

		VkPhysicalDeviceRayQueryFeaturesKHR rqFeatures{};
		rqFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		rqFeatures.rayQuery = VK_TRUE;

		std::vector<const char*> enabledExtensions = m_createInfo.deviceExtensions;
		if (m_rayQuerySupported)
		{
			enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
			enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
			enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

			sync2Features.pNext = &bdaFeatures;
			bdaFeatures.pNext = &asFeatures;
			asFeatures.pNext = &rqFeatures;
		}

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
		dynamicRenderingFeatures.pNext = &sync2Features;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.pNext = &dynamicRenderingFeatures;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		Logger::Log<SUCCESS>("Vulkan logical device: ok");
		if (m_rayQuerySupported)
		{
			m_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_device, "vkCreateAccelerationStructureKHR"));
			m_vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_device, "vkDestroyAccelerationStructureKHR"));
			m_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureBuildSizesKHR"));
			m_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureDeviceAddressKHR"));
			m_vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_device, "vkCmdBuildAccelerationStructuresKHR"));

			if (!HasAccelerationStructureFunctions())
			{
				Logger::Log<WARNING>("Failed to load all VK_KHR_acceleration_structure function pointers; disabling ray query");
				m_rayQuerySupported = false;
			}
			else
			{
				Logger::Log<SUCCESS>("Vulkan hardware ray query (VK_KHR_ray_query + AS): ok");
			}
		}
		else
		{
			Logger::Log<INFO>("Vulkan hardware ray query: not supported");
		}
	}

	std::unique_ptr<Queue> Device::CreateGraphicsQueue() const
	{
		const uint32_t familyIndex = m_queueFamilyIndices.graphicsFamily.value();

		VkQueue graphicsQueue;
		vkGetDeviceQueue(m_device, familyIndex, 0, &graphicsQueue);

		Logger::Log<SUCCESS>("Vulkan graphics queue: ok");

		return Queue::Create({graphicsQueue, familyIndex});
	}

	std::unique_ptr<Queue> Device::CreatePresentQueue() const
	{
		const uint32_t familyIndex = m_queueFamilyIndices.presentFamily.value();

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

	VkFormat Device::FindDepthFormat() const
	{
		return FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	bool Device::IsSupportedFormatFeature(const VkFormat format, const VkFormatFeatureFlags features) const
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		return (props.optimalTilingFeatures & features) == features;
	}

	void Device::WaitIdle() const
	{
		if (m_device != VK_NULL_HANDLE)
			vkDeviceWaitIdle(m_device);
	}

	VkResult Device::CreateImageView(const VkImageViewCreateInfo& createInfo, VkImageView* imageView) const
	{
		return vkCreateImageView(m_device, &createInfo, nullptr, imageView);
	}

	void Device::DestroyImageView(VkImageView imageView) const
	{
		if (imageView != VK_NULL_HANDLE)
			vkDestroyImageView(m_device, imageView, nullptr);
	}

	VkResult Device::CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo, VkSampler* sampler) const
	{
		return vkCreateSampler(m_device, &samplerCreateInfo, nullptr, sampler);
	}

	void Device::DestroySampler(VkSampler sampler) const
	{
		if (sampler != VK_NULL_HANDLE)
			vkDestroySampler(m_device, sampler, nullptr);
	}

	VkResult Device::CreateShaderModule(const VkShaderModuleCreateInfo& createInfo, VkShaderModule* shaderModule) const
	{
		return vkCreateShaderModule(m_device, &createInfo, nullptr, shaderModule);
	}

	void Device::DestroyShaderModule(VkShaderModule shaderModule) const
	{
		if (shaderModule != VK_NULL_HANDLE)
			vkDestroyShaderModule(m_device, shaderModule, nullptr);
	}

	bool Device::HasRequiredRayQueryExtensions(const std::vector<VkExtensionProperties>& availableExtensions)
	{
		bool hasAS = false;
		bool hasRQ = false;
		bool hasDeferredHostOps = false;

		for (const auto& ext : availableExtensions)
		{
			if (std::strcmp(ext.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
				hasAS = true;
			else if (std::strcmp(ext.extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
				hasRQ = true;
			else if (std::strcmp(ext.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
				hasDeferredHostOps = true;
		}

		return hasAS && hasRQ && hasDeferredHostOps;
	}

	bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device) const noexcept
	{
		auto deviceExtensions = m_createInfo.deviceExtensions;

		uint32_t extensionCount;
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
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;

		uint32_t i = 0;
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

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
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

		VkPhysicalDeviceSynchronization2Features sync2Features{};
		sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.pNext = &sync2Features;

		VkPhysicalDeviceFeatures2 supportedFeatures2{};
		supportedFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		supportedFeatures2.pNext = &dynamicRenderingFeatures;
		vkGetPhysicalDeviceFeatures2(device, &supportedFeatures2);

		return queueFamiliesSupported && extensionsSupported && swapchainSupported && supportedFeatures2.features.samplerAnisotropy && supportedFeatures2.features.sampleRateShading && dynamicRenderingFeatures.dynamicRendering && sync2Features.synchronization2;
	}

	VkPhysicalDevice Device::FindSuitableDevice(const std::vector<VkPhysicalDevice>& devices) const
	{
		VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
		uint32_t		 bestScore = 0;

		for (const auto& device : devices)
		{
			if (!IsDeviceSuitable(device))
				continue;

			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(device, &props);

			uint32_t score = 1;
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;
			else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				score += 100;

			if (score > bestScore)
			{
				bestScore = score;
				bestDevice = device;
			}
		}

		if (bestDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU");

		return bestDevice;
	}

	std::string Device::GetVendorById(uint32_t vendorId)
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

	uint32_t Device::GetMinUniformBufferOffsetAlignment() const
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

	uint32_t Device::GetMaxTexture2DSize() const
	{
		return m_maxTexture2DSize;
	}

	uint32_t Device::GetMaxTexture3DSize() const
	{
		return m_maxTexture3DSize;
	}

	uint32_t Device::GetMaxTextureCubeSize() const
	{
		return m_maxTextureCubeSize;
	}

	bool Device::IsRayQuerySupported() const
	{
		return m_rayQuerySupported;
	}

	uint32_t Device::GetMinAccelerationStructureScratchOffsetAlignment() const
	{
		return m_minAccelerationStructureScratchOffsetAlignment;
	}

	bool Device::HasAccelerationStructureFunctions() const
	{
		return m_vkCreateAccelerationStructureKHR != nullptr &&
			   m_vkDestroyAccelerationStructureKHR != nullptr &&
			   m_vkGetAccelerationStructureBuildSizesKHR != nullptr &&
			   m_vkGetAccelerationStructureDeviceAddressKHR != nullptr &&
			   m_vkCmdBuildAccelerationStructuresKHR != nullptr;
	}

	VkDeviceAddress Device::GetBufferDeviceAddress(const VkBuffer buffer) const
	{
		VkBufferDeviceAddressInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		info.buffer = buffer;
		return vkGetBufferDeviceAddress(m_device, &info);
	}

	VkResult Device::CreateAccelerationStructure(const VkAccelerationStructureCreateInfoKHR& createInfo, VkAccelerationStructureKHR* outAs) const
	{
		return m_vkCreateAccelerationStructureKHR(m_device, &createInfo, nullptr, outAs);
	}

	void Device::DestroyAccelerationStructure(const VkAccelerationStructureKHR as) const
	{
		if (as != VK_NULL_HANDLE && m_vkDestroyAccelerationStructureKHR != nullptr)
			m_vkDestroyAccelerationStructureKHR(m_device, as, nullptr);
	}

	void Device::GetAccelerationStructureBuildSizes(const VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR& buildInfo, const uint32_t* maxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* outSizes) const
	{
		m_vkGetAccelerationStructureBuildSizesKHR(m_device, buildType, &buildInfo, maxPrimitiveCounts, outSizes);
	}

	VkDeviceAddress Device::GetAccelerationStructureDeviceAddress(const VkAccelerationStructureKHR as) const
	{
		VkAccelerationStructureDeviceAddressInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		info.accelerationStructure = as;
		return m_vkGetAccelerationStructureDeviceAddressKHR(m_device, &info);
	}

	void Device::CmdBuildAccelerationStructures(const VkCommandBuffer cmd, const uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const
	{
		m_vkCmdBuildAccelerationStructuresKHR(cmd, infoCount, pInfos, ppBuildRangeInfos);
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
