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

#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

namespace BetterThanNothing::RenderingApi
{
	class Instance;
	class Surface;
	class Queue;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR		capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>	presentModes;
	};

	struct DeviceCreateInfo
	{
		Instance* instance;
		Surface*  surface;

		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
		};
	};

	class Device
	{
	public:
		explicit Device(const DeviceCreateInfo& createInfo);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) = delete;
		Device& operator=(Device&& other) = delete;

		[[nodiscard]] static std::unique_ptr<Device> Create(const DeviceCreateInfo& createInfo);

	private:
		void PickPhysicalDevice();
		void GetPhysicalDeviceProperties();
		void CreateLogicalDevice();

	public:
		[[nodiscard]] std::unique_ptr<Queue> CreateGraphicsQueue() const;
		[[nodiscard]] std::unique_ptr<Queue> CreatePresentQueue() const;

		void WaitIdle() const;

		VkResult CreateImageView(const VkImageViewCreateInfo& createInfo, VkImageView* imageView) const;
		void	 DestroyImageView(VkImageView imageView) const;
		VkResult CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo, VkSampler* sampler) const;
		void	 DestroySampler(VkSampler sampler) const;

		VkResult CreateShaderModule(const VkShaderModuleCreateInfo& createInfo, VkShaderModule* shaderModule) const;
		void	 DestroyShaderModule(VkShaderModule shaderModule) const;


		[[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		[[nodiscard]] bool IsSupportedFormatFeature(VkFormat format, VkFormatFeatureFlags features) const;

	private:
		bool								  CheckDeviceExtensionSupport(VkPhysicalDevice device) const noexcept;
		[[nodiscard]] QueueFamilyIndices	  FindQueueFamilies(VkPhysicalDevice device) const;
		[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
		bool								  IsDeviceSuitable(VkPhysicalDevice device) const noexcept;
		[[nodiscard]] VkPhysicalDevice		  FindSuitableDevice(const std::vector<VkPhysicalDevice>& devices) const;
		[[nodiscard]] static std::string	  GetVendorById(uint32_t vendorId);

	public:
		[[nodiscard]] SwapChainSupportDetails GetSwapChainSupport() const;
		[[nodiscard]] QueueFamilyIndices	  GetQueueFamilyIndices() const;
		[[nodiscard]] std::string			  GetVendorName() const;
		[[nodiscard]] std::string			  GetDeviceName() const;
		[[nodiscard]] std::string			  GetApiVersion() const;
		[[nodiscard]] uint32_t				  GetMinUniformBufferOffsetAlignment() const;
		[[nodiscard]] VkSampleCountFlagBits	  GetMaxMsaaSamples() const;
		[[nodiscard]] float					  GetMaxAnisotropy() const;
		[[nodiscard]] uint32_t				  GetMaxTexture2DSize() const;
		[[nodiscard]] uint32_t				  GetMaxTexture3DSize() const;
		[[nodiscard]] uint32_t				  GetMaxTextureCubeSize() const;
		[[nodiscard]] VkDevice				  Handle() const;
		[[nodiscard]] VkPhysicalDevice		  PhysicalHandle() const;

	private:
		DeviceCreateInfo	  m_createInfo;
		VkDevice			  m_device = VK_NULL_HANDLE;
		VkPhysicalDevice	  m_physicalDevice = VK_NULL_HANDLE;
		QueueFamilyIndices	  m_queueFamilyIndices;
		std::string			  m_vendorName;
		std::string			  m_deviceName;
		std::string			  m_apiVersion;
		uint32_t			  m_minUniformBufferOffsetAlignment = 0;
		VkSampleCountFlagBits m_maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
		float				  m_maxAnisotropy = 1;
		uint32_t			  m_maxTexture2DSize = 0;
		uint32_t			  m_maxTexture3DSize = 0;
		uint32_t			  m_maxTextureCubeSize = 0;
	};
};
