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

#include "EVulkanBufferingMode.hpp"

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
			std::uint32_t apiVersion = VK_API_VERSION_1_3;

			bool enableValidationLayers = false;
			std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };

			std::string shadersFolderPath;

			union
			{
				EVulkanBufferingMode	bufferingMode = EVulkanBufferingMode::None;
				std::uint32_t					maxFramesInFlight;
			};

			bool enableVSync = true;
		} vkSettings;

		std::unique_ptr<CVulkanInstance>					vkInstance;
		std::unique_ptr<CVulkanSurface>						vkSurface;
		std::unique_ptr<CVulkanDevice>						vkDevice;
		std::unique_ptr<CVulkanQueue>						vkGraphicsQueue;
		std::unique_ptr<CVulkanQueue>						vkPresentQueue;
		std::unique_ptr<CVulkanMemoryAllocator>				vkMemoryAllocator;
		std::unique_ptr<CVulkanSwapChain>					vkSwapChain;
		std::unique_ptr<CVulkanRenderPass>					vkRenderPass;
		std::unique_ptr<CVulkanCommandPool>					vkCommandPool;
		std::unique_ptr<CVulkanPipeline>					vkPipeline;

		std::vector<VulkanFrame>							vkFrames;

		std::unique_ptr<CVulkanDescriptorPool>				vkDescriptorPool;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkGlobalDescriptorSetLayout;
		std::vector<std::unique_ptr<CVulkanDescriptorSet>>	vkGlobalDescriptorSets;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkDynamicDescriptorSetLayout;
		std::vector<std::unique_ptr<CVulkanDescriptorSet>>	vkDynamicDescriptorSets;

		std::unique_ptr<CVulkanDescriptorSetLayout>			vkSamplerDescriptorSetLayout;
	};
};
