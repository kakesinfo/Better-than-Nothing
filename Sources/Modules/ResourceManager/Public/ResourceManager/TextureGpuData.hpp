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

#include <Module_Vulkan/VulkanDescriptorSet.hpp>
#include <Module_Vulkan/VulkanImage.hpp>
#include <Module_Vulkan/VulkanImageView.hpp>
#include <Module_Vulkan/VulkanSampler.hpp>

namespace BetterThanNothing
{
	struct TextureGpuData
	{
		std::int32_t						 width;
		std::int32_t						 height;
		std::int32_t						 channels;
		std::int32_t						 size;
		std::uint32_t						 mipLevels;
		std::unique_ptr<VulkanImage>		 image;
		std::unique_ptr<VulkanImageView>	 imageView;
		std::unique_ptr<VulkanSampler>		 sampler;
		std::unique_ptr<VulkanDescriptorSet> descriptorSet;
	};
}
