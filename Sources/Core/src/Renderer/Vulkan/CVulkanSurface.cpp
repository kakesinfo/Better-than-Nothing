//
// Created by acoezard on 11/5/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanSurface::CVulkanSurface(CVulkanContext* context): m_context(context)
	{
		auto& pWindow = CWindowManager::GetWindow();

		if (glfwCreateWindowSurface(m_context->vkInstance->Handle(), pWindow->Handle(), nullptr, &m_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");

		LOG_SUCCESS("Vulkan surface: ok");
	}

	CVulkanSurface::~CVulkanSurface()
	{
		vkDestroySurfaceKHR(m_context->vkInstance->Handle(), m_surface, nullptr);
	}

	VkSurfaceKHR CVulkanSurface::Handle() const
	{
		if (m_surface == VK_NULL_HANDLE)
			throw std::runtime_error("CVulkanSurface::Handle() Vulkan surface is not initialized");
		return m_surface;
	}
};


