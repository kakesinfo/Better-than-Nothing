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
	class CVulkanImage;
	class CVulkanImageView;
	class CVulkanSampler;
	class CVulkanDescriptorSet;

	struct TextureGpuData
	{
		std::int32_t							width;
		std::int32_t							height;
		std::int32_t							channels;
		std::int32_t							size;
		std::uint32_t							mipLevels;

		std::unique_ptr<CVulkanImage>			pVkImage;
		std::unique_ptr<CVulkanImageView>		pVkImageView;
		std::unique_ptr<CVulkanSampler>			pVkSampler;

		std::unique_ptr<CVulkanDescriptorSet>	pVkDescriptorSet;
	};
}