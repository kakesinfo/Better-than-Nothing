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
	class VulkanInstance;
	class VulkanSurface;
	class VulkanDevice;
	class VulkanQueue;
	class VulkanMemoryAllocator;
	class VulkanSwapChain;
	class VulkanRenderPass;
	class VulkanCommandPool;
	class VulkanDescriptorSetLayout;
	class VulkanDescriptorPool;
	class VulkanDescriptorSet;
	class VulkanPipeline;
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
				std::uint32_t			maxFramesInFlight;
			};

			bool enableVSync = true;
		} vkSettings;

		std::unique_ptr<VulkanInstance>					vkInstance;
		std::unique_ptr<VulkanSurface>						vkSurface;
		std::unique_ptr<VulkanDevice>						vkDevice;
		std::unique_ptr<VulkanQueue>						vkGraphicsQueue;
		std::unique_ptr<VulkanQueue>						vkPresentQueue;
		std::unique_ptr<VulkanMemoryAllocator>				vkMemoryAllocator;
		std::unique_ptr<VulkanSwapChain>					vkSwapChain;
		std::unique_ptr<VulkanRenderPass>					vkRenderPass;
		std::unique_ptr<VulkanCommandPool>					vkCommandPool;
		std::unique_ptr<VulkanPipeline>					vkPipeline;

		std::vector<VulkanFrame>							vkFrames;

		std::unique_ptr<VulkanDescriptorPool>				vkDescriptorPool;

		std::unique_ptr<VulkanDescriptorSetLayout>			vkGlobalDescriptorSetLayout;
		std::vector<std::unique_ptr<VulkanDescriptorSet>>	vkGlobalDescriptorSets;

		std::unique_ptr<VulkanDescriptorSetLayout>			vkDynamicDescriptorSetLayout;
		std::vector<std::unique_ptr<VulkanDescriptorSet>>	vkDynamicDescriptorSets;

		std::unique_ptr<VulkanDescriptorSetLayout>			vkSamplerDescriptorSetLayout;
	};
};
