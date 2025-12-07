//
// Created by acoezard on 11/5/24.
//

#pragma once

#include "vulkan/vulkan.h"

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
