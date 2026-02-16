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

#include <RenderingApi/Deprecated/VulkanContext.hpp>
#include <RenderingApi/Deprecated/VulkanFrame.hpp>
#include <RenderingApi/Deprecated/VulkanUniformBuffer.hpp>

namespace BetterThanNothing
{
	class DynamicUniformProvider
	{
	public:
		explicit DynamicUniformProvider(VulkanContext* context, uint32_t maxObjects);
		~DynamicUniformProvider() = default;

		template <typename T>
		T* Get(const uint32_t frameIndex, const uint32_t objectIndex) const
		{
			char* base = static_cast<char*>(m_context->frames[frameIndex].dynamicUniformBuffer->GetData());
			return reinterpret_cast<T*>(base + objectIndex * m_uniformAlignment);
		}

		uint32_t GetOffset(const uint32_t objectIndex) const { return objectIndex * m_uniformAlignment; }

	private:
		uint32_t CalculateUniformAlignment() const;

		VulkanContext* m_context = nullptr;
		uint32_t	   m_maxObjects;
		uint32_t	   m_uniformAlignment;
	};
};
