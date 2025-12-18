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

	class CVulkanSurface
	{
	public:
		explicit CVulkanSurface(VulkanContext* context);
		~CVulkanSurface();

		CVulkanSurface(const CVulkanSurface&) = delete;
		CVulkanSurface& operator=(const CVulkanSurface&) = delete;
		CVulkanSurface(CVulkanSurface&& other) = delete;
		CVulkanSurface& operator=(CVulkanSurface&& other) = delete;

		[[nodiscard]]
		VkSurfaceKHR Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	};
};
