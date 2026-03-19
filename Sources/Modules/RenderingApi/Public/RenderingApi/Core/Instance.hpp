/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace BetterThanNothing::RenderingApi
{
	struct InstanceCreateInfo
	{
		bool					 enableValidation = true;
		std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
		uint32_t				 apiVersion = VK_API_VERSION_1_4;
	};

	class Instance
	{
	public:
		explicit Instance(InstanceCreateInfo createInfo);
		~Instance();

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance(Instance&& other) = delete;
		Instance& operator=(Instance&& other) = delete;

		[[nodiscard]] static std::unique_ptr<Instance> Create(const InstanceCreateInfo& createInfo);

	private:
		[[nodiscard]] bool					   CheckValidationLayerSupport() const noexcept;
		[[nodiscard]] std::vector<const char*> GetRequiredExtensions() const noexcept;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) noexcept;

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) noexcept;
		void		CreateDebugMessenger();
		void		DestroyDebugMessenger() const;

		[[nodiscard]] static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger) noexcept;
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* allocator) noexcept;

	public:
		[[nodiscard]] uint32_t	 GetApiVersion() const noexcept;
		[[nodiscard]] bool		 IsValidationEnabled() const noexcept;
		[[nodiscard]] VkInstance Handle() const;

	private:
		InstanceCreateInfo		 m_createInfo;
		VkInstance				 m_instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

		bool m_enableValidationLayers = false;
	};
};
