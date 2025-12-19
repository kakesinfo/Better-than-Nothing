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

#include "module_vulkan.hpp"

#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	VulkanSurface::VulkanSurface(VulkanContext* context): m_context(context)
	{
		auto& window = WindowManager::GetWindow();

		if (glfwCreateWindowSurface(m_context->vkInstance->Handle(), window->Handle(), nullptr, &m_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");

		Logger::Log<Success>("Vulkan surface: ok");
	}

	VulkanSurface::~VulkanSurface()
	{
		vkDestroySurfaceKHR(m_context->vkInstance->Handle(), m_surface, nullptr);
	}

	VkSurfaceKHR VulkanSurface::Handle() const
	{
		if (m_surface == VK_NULL_HANDLE)
			throw std::runtime_error("CVulkanSurface::Handle() Vulkan surface is not initialized");
		return m_surface;
	}
};


