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

#include <RenderingDevice/VertexLayout.hpp>

#include <stdexcept>

namespace BetterThanNothing
{
	static constexpr uint32_t Internal_GetVertexTypeSize(const VertexType type)
	{
		switch (type)
		{
			case VertexType::Float:
				return 4;
			case VertexType::Float2:
				return 4 * 2;
			case VertexType::Float3:
				return 4 * 3;
			case VertexType::Float4:
				return 4 * 4;
			case VertexType::Int:
				return 4;
			case VertexType::Int2:
				return 4 * 2;
			case VertexType::Int3:
				return 4 * 3;
			case VertexType::Int4:
				return 4 * 4;
			default:
				throw std::runtime_error("Invalid vertex type");
		}
	}

	VertexLayout::VertexLayout(const std::initializer_list<VertexType>& attributeTypes)
	{
		uint32_t currentOffset = 0;
		uint32_t currentLocation = 0;

		m_attributes.clear();
		m_attributes.reserve(attributeTypes.size());
		for (const auto& type : attributeTypes)
		{
			VertexLayoutAttribute attribute{};
			attribute.type = type;
			attribute.binding = 0;
			attribute.location = currentLocation;
			attribute.offset = currentOffset;

			m_attributes.push_back(attribute);

			currentOffset += Internal_GetVertexTypeSize(type);
			currentLocation++;
		}

		m_stride = currentOffset;
	}

	const std::vector<VertexLayoutAttribute>& VertexLayout::GetAttributes() const
	{
		return m_attributes;
	}

	uint32_t VertexLayout::GetStride() const
	{
		return m_stride;
	}
}
