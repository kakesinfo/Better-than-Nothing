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

#include "Types.hpp"
#include "Handle.hpp"

namespace BetterThanNothing
{
	struct TextureFromDataDescription
	{
		void*	 data = nullptr;
		size_t	 dataSize = 0;
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t channels = 4;
	};

	struct CopyDataToTextureImageDescription
	{
		Handle destination;
		void*		  data{};
		std::uint32_t dataSize{};
		VkFormat	  format{};
		VkImageLayout oldLayout{};
		VkImageLayout newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	};

	struct TextureImageDescription
	{
		Handle		  destination;
		std::uint32_t		  width{};
		std::uint32_t		  height{};
		std::uint32_t		  depth = 1;
		std::uint8_t		  mipLevels = 1;
		VkFormat			  format{};
		VkImageLayout		  initialLayout{};
		VkImageUsageFlags	  usage{};
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		bool				  dedicated = false;
	};

	struct TextureImageViewDescription
	{
		Handle	   destination;
		VkImageAspectFlags aspectFlags{};
	};
}
