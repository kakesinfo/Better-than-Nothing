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

#include <RenderingApi/VulkanContext.hpp>
#include <RenderingApi/VulkanFrame.hpp>
#include <RenderingApi/VulkanUniformBuffer.hpp>

namespace BetterThanNothing
{
	class DynamicUniformProvider
	{
	public:
		explicit DynamicUniformProvider(VulkanContext* context, std::uint32_t maxObjects);
		~DynamicUniformProvider() = default;

		template <typename T>
		T* Get(const std::uint32_t frameIndex, const std::uint32_t objectIndex) const
		{
			char* base = static_cast<char*>(m_context->vkFrames[frameIndex].dynamicUniformBuffer->GetData());
			return reinterpret_cast<T*>(base + objectIndex * m_uniformAlignment);
		}

		std::uint32_t GetOffset(const std::uint32_t objectIndex) const { return objectIndex * m_uniformAlignment; }

	private:
		std::uint32_t CalculateUniformAlignment() const;

		VulkanContext* m_context = nullptr;
		std::uint32_t  m_maxObjects;
		std::uint32_t  m_uniformAlignment;
	};
};
