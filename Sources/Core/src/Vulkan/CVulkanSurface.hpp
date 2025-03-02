//
// Created by acoezard on 11/5/24.
//

#pragma once

#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	class CApplicationContext;

	class CVulkanSurface
	{
		CApplicationContext* m_context = nullptr;

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	public:
		explicit CVulkanSurface(CApplicationContext* context);
		~CVulkanSurface();

		CVulkanSurface(const CVulkanSurface&) = delete;
		CVulkanSurface& operator=(const CVulkanSurface&) = delete;
		CVulkanSurface(CVulkanSurface&& other) = delete;
		CVulkanSurface& operator=(CVulkanSurface&& other) = delete;

		[[nodiscard]]
		VkSurfaceKHR Handle() const;
	};
};
