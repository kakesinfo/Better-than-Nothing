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

#include "../../../../Base/Public/Module_Base/EBufferingMode.hpp"

namespace BetterThanNothing
{
	class Instance;
	class Surface;
	class Device;
	class Queue;
	class Allocator;
	class SwapChain;
	class CommandPool;
	class VulkanDescriptorSetLayout;
	class DescriptorPool;
	class VulkanDescriptorSet;
	class Pipeline;
	struct VulkanFrame;

	/// @deprecated Will be removed
	struct VulkanContext
	{
		struct
		{
			union
			{
				EBufferingMode bufferingMode = EBufferingMode::Double;
				uint32_t	   maxFramesInFlight;
			};
		} settings;

		Instance*				  instance;
		Surface*				  surface;
		Device*					  device;
		Queue*					  graphicsQueue;
		Queue*					  presentQueue;
		Allocator*				  allocator;
		SwapChain*				  swapChain;
		CommandPool*			  commandPool;
		std::unique_ptr<Pipeline> defaultPipeline;

		std::vector<VulkanFrame> frames;

		std::unique_ptr<DescriptorPool> descriptorPool;

		std::unique_ptr<VulkanDescriptorSetLayout>		  vkGlobalDescriptorSetLayout;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> vkGlobalDescriptorSets;

		std::unique_ptr<VulkanDescriptorSetLayout>		  vkDynamicDescriptorSetLayout;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> vkDynamicDescriptorSets;

		std::unique_ptr<VulkanDescriptorSetLayout> vkSamplerDescriptorSetLayout;

		std::unique_ptr<VulkanDescriptorSetLayout>		  vkPointLightDescriptorLayout;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> vkPointLightDescriptorSets;
	};
};
