//
// Created by acoezard on 11/5/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanSurface::CVulkanSurface(CApplicationContext* context): m_context(context)
	{
		if (glfwCreateWindowSurface(m_context->GetVulkanInstance()->Handle(), m_context->GetWindow()->Handle(), nullptr, &m_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");

		LOG_SUCCESS("Vulkan surface: ok");
	}

	CVulkanSurface::~CVulkanSurface()
	{
		vkDestroySurfaceKHR(m_context->GetVulkanInstance()->Handle(), m_surface, nullptr);
	}

	VkSurfaceKHR CVulkanSurface::Handle() const
	{
		if (m_surface == VK_NULL_HANDLE)
			throw std::runtime_error("CVulkanSurface::Handle() Vulkan surface is not initialized");
		return m_surface;
	}
};


