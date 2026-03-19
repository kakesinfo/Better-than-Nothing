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

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace BetterThanNothing::RenderingApi
{
	struct Texture
	{
		VkImage			   value = VK_NULL_HANDLE;
		VmaAllocation	   allocation = VK_NULL_HANDLE;
		VkImageView		   defaultView = VK_NULL_HANDLE;
		std::uint32_t	   width = 0;
		std::uint32_t	   height = 0;
		std::uint8_t	   mipLevels = 1;
		VkFormat		   format = VK_FORMAT_UNDEFINED;
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	};
}
