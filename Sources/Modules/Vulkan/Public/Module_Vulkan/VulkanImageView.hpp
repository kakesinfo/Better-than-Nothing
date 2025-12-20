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
	struct VulkanContext;

	class VulkanImageView
	{
	public:
		explicit VulkanImageView(VulkanContext* context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, std::uint32_t mipLevels);
		~VulkanImageView();

		VulkanImageView(const VulkanImageView&) = delete;
		VulkanImageView& operator=(const VulkanImageView&) = delete;
		VulkanImageView(VulkanImageView&& other) = delete;
		VulkanImageView& operator=(VulkanImageView&& other) = delete;

		[[nodiscard]]
		VkImageView Handle() const { return m_imageView; }

	private:
		VulkanContext* m_context = nullptr;
		VkImageView m_imageView = VK_NULL_HANDLE;
	};
};
