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

namespace BetterThanNothing
{
	struct VulkanContext;

	/// @deprecated Will be replaced by SamplerHandle
	class VulkanSampler
	{
	public:
		explicit VulkanSampler(VulkanContext* context, const VkSamplerCreateInfo* samplerCreateInfo);
		~VulkanSampler();

		VulkanSampler(const VulkanSampler&) = delete;
		VulkanSampler& operator=(const VulkanSampler&) = delete;

		[[nodiscard]]
		VkSampler Handle() const;

	private:
		VulkanContext* m_context = nullptr;
		VkSampler	   m_sampler = VK_NULL_HANDLE;
	};
}
