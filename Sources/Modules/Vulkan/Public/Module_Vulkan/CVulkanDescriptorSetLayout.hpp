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

	class CVulkanDescriptorSetLayoutError final : public std::runtime_error
	{
	public:
		explicit CVulkanDescriptorSetLayoutError(const std::string& message)
			: std::runtime_error(message) {}
	};

	class CVulkanDescriptorSetLayout
	{
	public:
		explicit CVulkanDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings, VulkanContext* context);
		~CVulkanDescriptorSetLayout();

		CVulkanDescriptorSetLayout(const CVulkanDescriptorSetLayout&) = delete;
		CVulkanDescriptorSetLayout& operator=(const CVulkanDescriptorSetLayout&) = delete;
		CVulkanDescriptorSetLayout(CVulkanDescriptorSetLayout&& other) = delete;
		CVulkanDescriptorSetLayout& operator=(CVulkanDescriptorSetLayout&& other) = delete;

		[[nodiscard]]
		VkDescriptorSetLayout Handle() const;

	private:
		VulkanContext* m_context;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	};
}