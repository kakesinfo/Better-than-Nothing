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
	namespace RenderingApi
	{
		class Instance;
		class Surface;
		class Device;
		class Queue;
		class Allocator;
		class SwapChain;
		class CommandPool;
		class Pipeline;
	class DescriptorPool;
	}

	class VulkanDescriptorSetLayout;
	class VulkanDescriptorSet;
	struct VulkanFrame;

	/// @deprecated Will be removed
	struct VulkanContext
	{
		struct
		{
			uint32_t maxFramesInFlight = 2;
		} settings;

		RenderingApi::Instance*					instance;
		RenderingApi::Surface*					surface;
		RenderingApi::Device*					device;
		RenderingApi::Queue*					graphicsQueue;
		RenderingApi::Queue*					presentQueue;
		RenderingApi::Allocator*				allocator;
		RenderingApi::SwapChain*				swapChain;
		RenderingApi::CommandPool*				commandPool;
		std::unique_ptr<RenderingApi::Pipeline> defaultPipeline;

		std::vector<VulkanFrame> frames;

		RenderingApi::DescriptorPool* descriptorPool;

		std::vector<std::unique_ptr<VulkanDescriptorSet>> vkGlobalDescriptorSets;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> vkPointLightDescriptorSets;
	};
};
