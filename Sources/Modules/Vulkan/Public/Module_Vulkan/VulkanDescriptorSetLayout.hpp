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

	class VulkanDescriptorSetLayoutError final : public std::runtime_error
	{
	public:
		explicit VulkanDescriptorSetLayoutError(const std::string& message)
			: std::runtime_error(message) {}
	};

	class VulkanDescriptorSetLayout
	{
	public:
		explicit VulkanDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, VulkanContext* context);
		~VulkanDescriptorSetLayout();

		VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
		VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;
		VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) = delete;
		VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) = delete;

		[[nodiscard]]
		VkDescriptorSetLayout Handle() const;

	private:
		VulkanContext* m_context;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	};
}