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
 *	GNU General Public Licens                e for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <Data/Graphics/MeshData.hpp>
#include <Data/Graphics/VertexData.hpp>

namespace BetterThanNothing
{
	class MeshBuilder
	{
	public:
		MeshBuilder() = default;
		MeshBuilder(const MeshBuilder&) = delete;
		MeshBuilder& operator=(const MeshBuilder&) = delete;
		~MeshBuilder() = default;

		void AddVertex(float x, float y, float z, float r, float g, float b, float u, float v, float nx, float ny,
					   float nz);
		void AddVertex(Vector3 position, Vector3 color, Vector2 uv, Vector3 normal);
		void AddVertex(const VertexData& vertexData);

		void AddIndices(const std::vector<uint32_t>& newIndices);
		void AddIndex(uint32_t index);

		MeshData Build();

	private:
		MeshData meshData;
	};
}
