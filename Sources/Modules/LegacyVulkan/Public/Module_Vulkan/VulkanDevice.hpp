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

	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t> graphicsFamily;
		std::optional<std::uint32_t> presentFamily;

		bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR		capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>	presentModes;
	};

	class VulkanDevice
	{
	public:
		explicit VulkanDevice(VulkanContext* context);
		~VulkanDevice();

		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice& operator=(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&& other) = delete;
		VulkanDevice& operator=(VulkanDevice&& other) = delete;

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
									 VkFormatFeatureFlags features) const;

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
		std::uint32_t GetMinUniformBufferOffsetAlignment() const;

		[[nodiscard]]
		float GetMaxAnisotropy() const;

		[[nodiscard]]
		VkSampleCountFlagBits GetMaxMsaaSamples() const;

		[[nodiscard]]
		SwapChainSupportDetails GetSwapChainSupport() const;

		[[nodiscard]]
		QueueFamilyIndices GetQueueFamilies() const;

		void WaitIdle() const;

	private:
		VkPhysicalDevice		FindSuitableDevice(const std::vector<VkPhysicalDevice>& devices) const;
		bool					IsDeviceSuitable(VkPhysicalDevice device) const noexcept;
		bool					CheckDeviceExtensionSupport(VkPhysicalDevice device) const noexcept;
		QueueFamilyIndices		FindQueueFamilies(VkPhysicalDevice device) const;
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

		[[nodiscard]]
		std::string GetVendorById(std::uint32_t vendorId) const;

		void PickPhysicalDevice();
		void GetPhysicalDeviceProperties();
		void CreateLogicalDevice();

		VulkanContext* m_context = nullptr;

		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice		 m_logicalDevice = VK_NULL_HANDLE;

		std::string			  m_vendorName;
		std::string			  m_deviceName;
		std::string			  m_apiVersion;
		std::uint32_t		  m_minUniformBufferOffsetAlignment = 0;
		VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
		float				  m_maxAnisotropy = 1;
	};
};
