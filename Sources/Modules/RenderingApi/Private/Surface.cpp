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

#include "RenderingApi.hpp"

#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	Surface::Surface(VulkanContext* context) : m_context(context)
	{
		auto& window = WindowManager::GetWindow();

		if (!SDL_Vulkan_CreateSurface(window->Handle(), m_context->vkInstance->Handle(), nullptr, &m_surface))
			throw std::runtime_error(std::format("Failed to create Vulkan surface: {}", SDL_GetError()));

		Logger::Log<SUCCESS>("Vulkan surface: ok");
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(m_context->vkInstance->Handle(), m_surface, nullptr);
	}

	VkSurfaceKHR Surface::Handle() const
	{
		if (m_surface == VK_NULL_HANDLE)
			throw std::runtime_error("CVulkanSurface::Handle() Vulkan surface is not initialized");
		return m_surface;
	}
};
