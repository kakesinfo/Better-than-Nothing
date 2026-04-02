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

#include "Common/EShaderType.hpp"

#include <initializer_list>
#include <vector>

namespace BetterThanNothing
{
	enum class VertexType : uint8_t
	{
		NONE = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Int,
		Int2,
		Int3,
		Int4,
	};

	static uint32_t GetVertexTypeSize(const VertexType type)
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
				return 0;
		}
	}

	struct VertexLayoutAttribute
	{
		VertexType type;
		uint32_t   binding;
		uint32_t   location;
		uint32_t   offset;
	};

	class VertexLayout
	{
	public:
		VertexLayout(const std::initializer_list<VertexType>& attributeTypes);
		~VertexLayout() = default;

		const std::vector<VertexLayoutAttribute>& GetAttributes() const;
		uint32_t GetStride() const;

	private:
		std::vector<VertexLayoutAttribute> m_attributes;
		uint32_t						   m_stride;
	};
}
