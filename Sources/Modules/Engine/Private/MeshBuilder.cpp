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
 *	GNU General Public Licens                e for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Module_Engine/MeshBuilder.hpp"

namespace BetterThanNothing
{
	void MeshBuilder::AddVertex(const float x, const float y, const float z, const float r, const float g, const float b, const float u, const float v, const float nx, const float ny, const float nz)
	{
		AddVertex({x, y, z}, {r, g, b}, {u, v}, {nx, ny, nz});
	}

	void MeshBuilder::AddVertex(const glm::vec3 position, const glm::vec3 color, const glm::vec2 uv, const glm::vec3 normal)
	{
		AddVertex({position, color, uv, normal});
	}

	void MeshBuilder::AddVertex(const VertexData vertexData)
	{
		meshData.vertices.push_back(vertexData);
	}

	void MeshBuilder::AddIndices(const std::vector<std::uint32_t>& newIndices)
	{
		meshData.indices.insert(meshData.indices.end(), newIndices.begin(), newIndices.end());
	}

	void MeshBuilder::AddIndex(const std::uint32_t index)
	{
		meshData.indices.push_back(index);
	}

	MeshData MeshBuilder::Build()
	{
		if (meshData.vertices.empty() || meshData.indices.empty())
			throw std::runtime_error("MeshBuilder: Cannot build mesh with no vertices or indices.");
		return meshData;
	}
}