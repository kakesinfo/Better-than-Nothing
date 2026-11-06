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

#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

namespace BetterThanNothing
{
	struct VertexData
	{
		Vector3 position;
		Vector3 color;
		Vector2 uv;
		Vector3 normal;
		Vector4 tangent{0.0f, 0.0f, 0.0f, 1.0f};
	};

	struct VertexHash
	{
		size_t operator()(VertexData const& v) const noexcept
		{
			auto h = std::hash<float>{}(v.position.GetX());
			h = h * 31 + std::hash<float>{}(v.position.GetY());
			h = h * 31 + std::hash<float>{}(v.position.GetZ());
			h = h * 31 + std::hash<float>{}(v.uv.GetX());
			h = h * 31 + std::hash<float>{}(v.uv.GetY());
			h = h * 31 + std::hash<float>{}(v.color.GetX());
			h = h * 31 + std::hash<float>{}(v.color.GetY());
			h = h * 31 + std::hash<float>{}(v.color.GetZ());
			h = h * 31 + std::hash<float>{}(v.normal.GetX());
			h = h * 31 + std::hash<float>{}(v.normal.GetY());
			h = h * 31 + std::hash<float>{}(v.normal.GetZ());
			h = h * 31 + std::hash<float>{}(v.tangent.GetX());
			h = h * 31 + std::hash<float>{}(v.tangent.GetY());
			h = h * 31 + std::hash<float>{}(v.tangent.GetZ());
			h = h * 31 + std::hash<float>{}(v.tangent.GetW());
			return h;
		}
	};

	struct VertexEqual
	{
		bool operator()(VertexData const& a, VertexData const& b) const noexcept
		{
			return a.position.Equals(b.position) && a.color.Equals(b.color) && a.uv.Equals(b.uv) &&
				   a.normal.Equals(b.normal) && a.tangent.Equals(b.tangent);
		}
	};
}
