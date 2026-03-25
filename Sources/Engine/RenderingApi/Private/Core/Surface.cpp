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

#include "RenderingApi.hpp"

#include <Common/Logger.hpp>
#include <Platform/WindowManager.hpp>
#include <Platform/Window.hpp>
#include <SDL3/SDL_vulkan.h>

namespace BetterThanNothing::RenderingApi
{
	Surface::Surface(const SurfaceCreateInfo& createInfo) : m_createInfo(createInfo)
	{
		auto window = static_cast<SDL_Window*>(m_createInfo.window);

		if (!SDL_Vulkan_CreateSurface(window, m_createInfo.instance->Handle(), nullptr, &m_surface))
			throw std::runtime_error(std::format("Failed to create Vulkan surface: {}", SDL_GetError()));
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(m_createInfo.instance->Handle(), m_surface, nullptr);
	}

	std::unique_ptr<Surface> Surface::Create(const SurfaceCreateInfo& createInfo)
	{
		return std::make_unique<Surface>(createInfo);
	}

	VkSurfaceKHR Surface::Handle() const
	{
		return m_surface;
	}
};
