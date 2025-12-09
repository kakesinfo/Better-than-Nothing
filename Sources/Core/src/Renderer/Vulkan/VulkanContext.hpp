//
// Created by kakesinfo on 27/11/2025.
//

#pragma once

#include <vector>

#include "CVulkanDescriptorPool.hpp"

namespace BetterThanNothing
{
	class CVulkanInstance;
	class CVulkanSurface;
	class CVulkanDevice;
	class CVulkanQueue;
	class CVulkanMemoryAllocator;
	class CVulkanSwapChain;
	class CVulkanRenderPass;
	class CVulkanCommandPool;
	class CVulkanDescriptorSetLayout;
	class CVulkanDescriptorPool;
	class CVulkanDescriptorSet;
	class CVulkanPipeline;
	struct VulkanFrame;

	struct VulkanContext
	{
		struct
		{
			uint32 apiVersion = VK_API_VERSION_1_3;

			bool enableValidationLayers = false;
			std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

			std::string shadersFolderPath;

			EVulkanBufferingMode bufferingMode = EVulkanBufferingMode::None;
			uint32 maxFramesInFlight = 0;

			bool enableVSync = true;
		} vkSettings;

		std::unique_ptr<CVulkanInstance>					vkInstance = nullptr;
		std::unique_ptr<CVulkanSurface>						vkSurface = nullptr;
		std::unique_ptr<CVulkanDevice>						vkDevice = nullptr;
		std::unique_ptr<CVulkanQueue>						vkGraphicsQueue = nullptr;
		std::unique_ptr<CVulkanQueue>						vkPresentQueue = nullptr;
		std::unique_ptr<CVulkanMemoryAllocator>				vkMemoryAllocator = nullptr;
		std::unique_ptr<CVulkanSwapChain>					vkSwapChain = nullptr;
		std::unique_ptr<CVulkanRenderPass>					vkRenderPass = nullptr;
		std::unique_ptr<CVulkanCommandPool>					vkCommandPool = nullptr;
		std::unique_ptr<CVulkanPipeline>					vkPipeline = nullptr;

		std::vector<VulkanFrame>							vkFrames;

		std::unique_ptr<CVulkanDescriptorPool>				vkDescriptorPool = nullptr;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkGlobalDescriptorSetLayout = nullptr;
		std::vector<std::unique_ptr<CVulkanDescriptorSet>>	vkGlobalDescriptorSets;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkDynamicDescriptorSetLayout = nullptr;
		std::vector<std::unique_ptr<CVulkanDescriptorSet>>	vkDynamicDescriptorSets;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkSamplerDescriptorSetLayout = nullptr;
		std::vector<std::unique_ptr<CVulkanDescriptorSet>>	vkSamplerDescriptorSets;
	};
};
