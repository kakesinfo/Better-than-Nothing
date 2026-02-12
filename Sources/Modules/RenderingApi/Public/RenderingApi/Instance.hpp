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

#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class Instance
	{
	public:
		explicit Instance(VulkanContext* context);
		~Instance();

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance(Instance&& other) = delete;
		Instance& operator=(Instance&& other) = delete;

		[[nodiscard]]
		VkInstance Handle() const;

	private:
		bool CheckValidationLayerSupport() const noexcept;

		[[nodiscard]]
		std::vector<const char*> GetRequiredExtensions() const noexcept;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT				messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
															void* userData) noexcept;

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) noexcept;
		void CreateDebugMessenger();
		void DestroyDebugMessenger();

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
											  const VkAllocationCallbacks* allocator,
											  VkDebugUtilsMessengerEXT*	   debugMessenger) noexcept;

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
										   const VkAllocationCallbacks* allocator) noexcept;

		VulkanContext*			 m_context = nullptr;
		VkInstance				 m_instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	};
};
