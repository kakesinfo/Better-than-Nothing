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

namespace BetterThanNothing
{
	struct VertexData
	{
		Vector3 position;
		Vector3 color;
		Vector2 uv;
		Vector3 normal;
	};

	static_assert(sizeof(VertexData) == 44, "VertexData must stay tightly packed (11 floats) to match VertexLayout");

	struct VertexHash
	{
		size_t operator()(VertexData const& v) const noexcept
		{
			auto hashFloat = [](const float f) noexcept { return std::hash<float>{}(f == 0.0f ? 0.0f : f); };
			auto combine = [](size_t& seed, const size_t val) noexcept {
				seed ^= val + 0x9e3779b9u + (seed << 6) + (seed >> 2);
			};

			size_t h = 0;
			combine(h, hashFloat(v.position.GetX()));
			combine(h, hashFloat(v.position.GetY()));
			combine(h, hashFloat(v.position.GetZ()));
			combine(h, hashFloat(v.color.GetX()));
			combine(h, hashFloat(v.color.GetY()));
			combine(h, hashFloat(v.color.GetZ()));
			combine(h, hashFloat(v.uv.GetX()));
			combine(h, hashFloat(v.uv.GetY()));
			combine(h, hashFloat(v.normal.GetX()));
			combine(h, hashFloat(v.normal.GetY()));
			combine(h, hashFloat(v.normal.GetZ()));
			return h;
		}
	};

	struct VertexEqual
	{
		bool operator()(VertexData const& a, VertexData const& b) const noexcept
		{
			return a.position.Equals(b.position) && a.color.Equals(b.color) && a.uv.Equals(b.uv) &&
				   a.normal.Equals(b.normal);
		}
	};
}
