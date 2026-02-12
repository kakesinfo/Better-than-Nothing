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

#include <Module_Base/Logger.hpp>

namespace BetterThanNothing
{
	Instance::Instance(InstanceCreateInfo  createInfo) : m_createInfo(std::move(createInfo))
	{
		const bool validationLayersAvailable = CheckValidationLayerSupport();
		const bool validationLayersEnabled = m_createInfo.enableValidation;

		if (validationLayersEnabled && !validationLayersAvailable)
			Logger::Log<WARNING>("Validation layers are enabled but not available on this system");

		m_enableValidationLayers = validationLayersEnabled & validationLayersAvailable;

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = m_createInfo.apiVersion;

		VkInstanceCreateInfo vkCreateInfo{};
		vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkCreateInfo.pApplicationInfo = &appInfo;

		// Prepare extensions
		const auto extensions = GetRequiredExtensions();
		vkCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
		vkCreateInfo.ppEnabledExtensionNames = extensions.data();

		// Prepare validation layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (validationLayersEnabled)
		{
			PopulateDebugMessengerCreateInfo(&debugCreateInfo);

			const auto& validationLayers = m_createInfo.validationLayers;
			vkCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
			vkCreateInfo.ppEnabledLayerNames = validationLayers.data();

			vkCreateInfo.pNext = &debugCreateInfo;
		}
		else
		{
			vkCreateInfo.enabledLayerCount = 0;
			vkCreateInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&vkCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
			throw std::runtime_error("failed to create instance!");

		Logger::Log<SUCCESS>("Vulkan instance: ok");

		CreateDebugMessenger();
	}

	Instance::~Instance()
	{
		DestroyDebugMessenger();
		vkDestroyInstance(m_instance, nullptr);
	}

	std::unique_ptr<Instance> Instance::Create(const InstanceCreateInfo& createInfo)
	{
		return std::make_unique<Instance>(createInfo);
	}

	bool Instance::CheckValidationLayerSupport() const noexcept
	{
		std::uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		return std::ranges::all_of(m_createInfo.validationLayers, [&availableLayers](const char* layerName) {
			return std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties& layerProperties) {
				return std::string_view(layerProperties.layerName) == layerName;
			});
		});
	}

	std::vector<const char*> Instance::GetRequiredExtensions() const noexcept
	{
		std::uint32_t	   sdlExtensionCount = 0;
		const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

		std::vector extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

		if (m_createInfo.enableValidation)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT	   messageSeverity,
														   VkDebugUtilsMessageTypeFlagsEXT			   messageType,
														   const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
														   void* userData) noexcept
	{
		(void)messageType;
		(void)userData;

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Logger::Log<ERROR>("validation layer: {}", callbackData->pMessage);
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			Logger::Log<WARNING>("validation layer: {}", callbackData->pMessage);
		}

		return VK_FALSE;
	}

	void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) noexcept
	{
		createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo->pNext = nullptr;
		createInfo->flags = 0;
		createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo->pfnUserCallback = DebugCallback;
		createInfo->pUserData = nullptr;
	}

	void Instance::CreateDebugMessenger()
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");

		if (!m_enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(&createInfo);

		if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}

		Logger::Log<SUCCESS>("Debug Messenger: ok");
	}

	void Instance::DestroyDebugMessenger() const
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");

		if (!m_enableValidationLayers)
			return;

		if (m_debugMessenger != VK_NULL_HANDLE)
		{
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}
	}

	VkResult Instance::CreateDebugUtilsMessengerEXT(VkInstance								  instance,
													const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
													const VkAllocationCallbacks*			  allocator,
													VkDebugUtilsMessengerEXT*				  debugMessenger) noexcept
	{
		const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		if (function == nullptr)
			return VK_ERROR_EXTENSION_NOT_PRESENT;

		return function(instance, createInfo, allocator, debugMessenger);
	}

	void Instance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
												 const VkAllocationCallbacks* allocator) noexcept
	{
		const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		if (function != nullptr)
			function(instance, debugMessenger, allocator);
	}

	std::uint32_t Instance::GetApiVersion() const noexcept
	{
		return m_createInfo.apiVersion;
	}

	bool Instance::IsValidationEnabled() const noexcept
	{
		return m_enableValidationLayers;
	}

	VkInstance Instance::Handle() const
	{
		if (m_instance == VK_NULL_HANDLE)
			throw std::runtime_error("Vulkan instance is not initialized");
		return m_instance;
	}
}
