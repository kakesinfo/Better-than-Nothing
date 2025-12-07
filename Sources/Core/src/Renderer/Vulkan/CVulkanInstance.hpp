//
// Created by acoezard on 11/5/24.
//

#pragma once

#include <vector>

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanInstance
	{
	public:
		explicit CVulkanInstance(VulkanContext* context);
		~CVulkanInstance();

		CVulkanInstance(const CVulkanInstance&) = delete;
		CVulkanInstance& operator=(const CVulkanInstance&) = delete;
		CVulkanInstance(CVulkanInstance&& other) = delete;
		CVulkanInstance& operator=(CVulkanInstance&& other) = delete;

		[[nodiscard]]
		VkInstance Handle() const;

	private:
		bool _checkValidationLayerSupport() noexcept;

		[[nodiscard]]
		std::vector<const char*> _getRequiredExtensions() noexcept;

		static VKAPI_ATTR VkBool32 VKAPI_CALL s_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) noexcept;

		void _populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) noexcept;
		void _createDebugMessenger();
		void _destroyDebugMessenger();

		VkResult _createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger
		) noexcept;

		void _destroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator
		) noexcept;

		VulkanContext* m_context = nullptr;
		VkInstance m_instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	};
};


