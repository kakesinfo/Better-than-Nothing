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
	class Instance;

	struct SurfaceCreateInfo
	{
		void*	  window;
		Instance* instance;
	};

	class Surface
	{
	public:
		explicit Surface(const SurfaceCreateInfo& createInfo);
		~Surface();

		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;
		Surface(Surface&& other) = delete;
		Surface& operator=(Surface&& other) = delete;

		[[nodiscard]] static std::unique_ptr<Surface> Create(const SurfaceCreateInfo& createInfo);

		[[nodiscard]] VkSurfaceKHR Handle() const;

	private:
		SurfaceCreateInfo m_createInfo;
		VkSurfaceKHR	  m_surface = VK_NULL_HANDLE;
	};
};
