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

#include "VertexData.hpp"
#include <vector>

namespace BetterThanNothing
{
	struct MeshData
	{
		std::vector<VertexData>	   vertices;
		std::vector<std::uint32_t> indices;

		std::uint32_t materialId;
		std::string	  texturePath;

		std::uint64_t id = 0; // TODO: move to loader
		explicit MeshData() : materialId(-1), id(s_globalIdCounter++) {}

	private:
		inline static std::uint64_t s_globalIdCounter = 0;
	};
}