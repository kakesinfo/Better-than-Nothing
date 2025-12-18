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

#include "Module_Engine/CPlaneMesh.hpp"

namespace BetterThanNothing
{
	CPlaneMesh::CPlaneMesh(const float size)
	{
		_generatePlane(size);
	}

	void CPlaneMesh::_generatePlane(const float size)
	{
		const std::int32_t subdivisions = static_cast<std::int32_t>(size);
		const float halfSize = size / 2.0f;

		for (std::int32_t x = 0; x < subdivisions; x++)
		{
			for (std::int32_t z = 0; z < subdivisions; z++)
			{
				const float posX = (static_cast<float>(x) / size) * size - halfSize;
				const float posZ = (static_cast<float>(z) / size) * size - halfSize;

				AddVertex(
					{posX, 0.0f, posZ},
					{1.0f, 1.0f, 1.0f},
					{static_cast<float>(x), static_cast<float>(z)},
					{0.0f, 1.0f, 0.0f}
				);

				if (x < subdivisions - 1 && z < subdivisions - 1)
				{
					const std::uint32_t topLeft = x * subdivisions + z;
					const std::uint32_t topRight = (x + 1) * subdivisions + z;
					const std::uint32_t bottomLeft = x * subdivisions + (z + 1);
					const std::uint32_t bottomRight = (x + 1) * subdivisions + (z + 1);

					// First triangle
					AddIndex(topLeft);
					AddIndex(bottomLeft);
					AddIndex(topRight);

					// Second triangle
					AddIndex(topRight);
					AddIndex(bottomLeft);
					AddIndex(bottomRight);
				}
			}
		}

	}
}