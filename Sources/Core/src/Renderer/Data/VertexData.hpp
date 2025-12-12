//
// Created by acoezard on 11/25/24.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 uv;
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
			return h;
		}
	};

	struct VertexEqual {
		bool operator()(VertexData const& a, VertexData const& b) const noexcept {
			return a.position == b.position
				&& a.color == b.color
				&& a.uv == b.uv;
		}
	};
}