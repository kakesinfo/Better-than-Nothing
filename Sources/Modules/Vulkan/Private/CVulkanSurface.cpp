#include "module_vulkan.hpp"

#include <Module_Platform/Manager/CWindowManager.hpp>
#include <Module_Platform/CWindow.hpp>

#include <Module_Base/CLogger.hpp>

namespace BetterThanNothing
{
	CVulkanSurface::CVulkanSurface(VulkanContext* context): m_context(context)
	{
		auto& pWindow = CWindowManager::GetWindow();

		if (glfwCreateWindowSurface(m_context->vkInstance->Handle(), pWindow->Handle(), nullptr, &m_surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");

		CLogger::Log<Success>("Vulkan surface: ok");
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


