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

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace BetterThanNothing
{
	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	struct VertexHash {
		std::size_t operator()(VertexData const& v) const noexcept {
			auto h = std::hash<float>{}(v.position.x);
			h = h * 31 + std::hash<float>{}(v.position.y);
			h = h * 31 + std::hash<float>{}(v.position.z);
			h = h * 31 + std::hash<float>{}(v.uv.x);
			h = h * 31 + std::hash<float>{}(v.uv.y);
			h = h * 31 + std::hash<float>{}(v.color.x);
			h = h * 31 + std::hash<float>{}(v.color.y);
			h = h * 31 + std::hash<float>{}(v.color.z);
			h = h * 31 + std::hash<float>{}(v.normal.x);
			h = h * 31 + std::hash<float>{}(v.normal.y);
			h = h * 31 + std::hash<float>{}(v.normal.z);
			return h;
		}
	};

	struct VertexEqual {
		bool operator()(VertexData const& a, VertexData const& b) const noexcept {
			return a.position == b.position
				&& a.color == b.color
				&& a.uv == b.uv
				&& a.normal == b.normal;
		}
	};
}