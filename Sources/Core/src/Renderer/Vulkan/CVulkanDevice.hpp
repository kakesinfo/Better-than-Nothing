//
// Created by acoezard on 11/6/24.
//

#pragma once

#include <optional>

#include "VulkanContext.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	struct QueueFamilyIndices
	{
		std::optional<uint32> graphicsFamily;
		std::optional<uint32> presentFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class CVulkanDevice
	{
	public:
		explicit CVulkanDevice(VulkanContext* context);
		~CVulkanDevice();

		CVulkanDevice(const CVulkanDevice&) = delete;
		CVulkanDevice& operator=(const CVulkanDevice&) = delete;
		CVulkanDevice(CVulkanDevice&& other) = delete;
		CVulkanDevice& operator=(CVulkanDevice&& other) = delete;

		[[nodiscard]]
		VkPhysicalDevice PhysicalHandle() const;

		[[nodiscard]]
		VkDevice Handle() const;

		[[nodiscard]]
		std::string GetVendorName() const;

		[[nodiscard]]
		std::string GetDeviceName() const;

		[[nodiscard]]
		std::string GetApiVersion() const;

		[[nodiscard]]
		SwapChainSupportDetails GetSwapChainSupport() const;

		[[nodiscard]]
		QueueFamilyIndices GetQueueFamilies() const;

		void WaitIdle() const;

	private:
		VkPhysicalDevice _findSuitableDevice(const std::vector<VkPhysicalDevice>& devices);
		bool _isDeviceSuitable(VkPhysicalDevice device) noexcept;
		bool _checkDeviceExtensionSupport(VkPhysicalDevice device) noexcept;
		QueueFamilyIndices _findQueueFamilies(VkPhysicalDevice device) const;
		SwapChainSupportDetails _querySwapChainSupport(VkPhysicalDevice device) const;

		[[nodiscard]]
		std::string _getVendorById(uint32 vendorId) const;

		void _pickPhysicalDevice();
		void _getPhysicalDeviceProperties();
		void _createLogicalDevice();


		VulkanContext* m_context = nullptr;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_logicalDevice = VK_NULL_HANDLE;

		std::string m_vendorName;
		std::string m_deviceName;
		std::string m_apiVersion;
	};
};
