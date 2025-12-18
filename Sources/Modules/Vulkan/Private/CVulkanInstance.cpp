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

#include <Module_Base/CLogger.hpp>

namespace BetterThanNothing
{
	CVulkanInstance::CVulkanInstance(VulkanContext* context): m_context(context)
	{
		bool validationLayersEnabled = context->vkSettings.enableValidationLayers;
		bool validationLayersAvailable = _checkValidationLayerSupport();

		if (validationLayersEnabled && !validationLayersAvailable)
			CLogger::Log<Warning>("Validation layers are enabled but not available on this system");

		validationLayersEnabled &= validationLayersAvailable;
		context->vkSettings.enableValidationLayers = validationLayersEnabled;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = context->vkSettings.apiVersion;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Prepare extensions
		const auto extensions = _getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Prepare validation layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (validationLayersEnabled)
		{
			_populateDebugMessengerCreateInfo(&debugCreateInfo);

			const auto& validationLayers = m_context->vkSettings.validationLayers;
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			createInfo.pNext = &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// Create instance
		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

		CLogger::Log<Success>("Vulkan instance: ok");

		_createDebugMessenger();
	}

	CVulkanInstance::~CVulkanInstance()
	{
		_destroyDebugMessenger();
		vkDestroyInstance(m_instance, nullptr);
	}

	std::vector<const char*> CVulkanInstance::_getRequiredExtensions() noexcept
	{
		std::uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_context->vkSettings.enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool CVulkanInstance::_checkValidationLayerSupport() noexcept
	{
		std::uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		const auto validationLayers = m_context->vkSettings.validationLayers;

		return std::ranges::any_of(availableLayers, [validationLayers](const VkLayerProperties& layerProperties) {
			return std::ranges::any_of(validationLayers, [&layerProperties](const char* layerName) {
				return layerProperties.layerName == std::string_view(layerName);
			});
		});
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL CVulkanInstance::s_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
																	void* pUserData) noexcept
	{
		(void) messageType;
		(void) pUserData;

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			CLogger::Log<Error>("validation layer: {}", pCallbackData->pMessage);
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			CLogger::Log<Warning>("validation layer: {}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	void CVulkanInstance::_populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) noexcept
	{
		createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo->pNext = nullptr;
		createInfo->flags = 0;
		createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo->pfnUserCallback = s_debugCallback;
		createInfo->pUserData = m_context;
	}

	void CVulkanInstance::_createDebugMessenger()
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");

		if (!m_context->vkSettings.enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		_populateDebugMessengerCreateInfo(&createInfo);

		if (_createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}

		CLogger::Log<Success>("Debug Messenger: ok");
	}

	void CVulkanInstance::_destroyDebugMessenger()
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");

		if (!m_context->vkSettings.enableValidationLayers)
			return;

		if (m_debugMessenger != VK_NULL_HANDLE) {
			_destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}
	}

	VkResult CVulkanInstance::_createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
															const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger) noexcept
	{
		auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (function == nullptr)
			return VK_ERROR_EXTENSION_NOT_PRESENT;

		return function(instance, createInfo, allocator, debugMessenger);
	}

	void CVulkanInstance::_destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
														 const VkAllocationCallbacks* allocator) noexcept
	{
		auto function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (function != nullptr) {
			function(instance, debugMessenger, allocator);
		}
	}

	VkInstance CVulkanInstance::Handle() const
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");
		return m_instance;
	}
}


