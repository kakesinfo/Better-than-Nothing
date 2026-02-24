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

#include "VulkanBuffer.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	/// @deprecated Will be replaced
	class VulkanUniformBuffer final : public VulkanBuffer
	{
	public:
		VulkanUniformBuffer(VulkanContext* context, VkDeviceSize size);

		template <typename T>
		void Update(const T& uboData)
		{
			Write(&uboData, sizeof(T));
		}

		template <typename T>
		void Update(const T& uboData, const uint32_t offset, const uint32_t alignment)
		{
			Write(&uboData, sizeof(T), offset, alignment);
		}

		void* GetData() const;
	};
}
