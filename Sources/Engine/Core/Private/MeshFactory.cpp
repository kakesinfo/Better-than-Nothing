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

#include "Core/MeshFactory.hpp"
#include "Core/MeshBuilder.hpp"

namespace BetterThanNothing
{
	MeshData MeshFactory::CreatePlane(const float size, const uint32_t subdivisions)
	{
		const uint32_t quads = subdivisions > 0 ? subdivisions : 1;
		const uint32_t vertsPerAxis = quads + 1;
		const float	   halfSize = size * 0.5f;
		const float	   invQuads = 1.0f / static_cast<float>(quads);

		MeshBuilder builder;
		for (uint32_t x = 0; x < vertsPerAxis; ++x)
		{
			for (uint32_t z = 0; z < vertsPerAxis; ++z)
			{
				const float u = static_cast<float>(x) * invQuads;
				const float v = static_cast<float>(z) * invQuads;
				const float posX = u * size - halfSize;
				const float posZ = v * size - halfSize;

				builder.AddVertex({posX, 0.0f, posZ}, {1.0f, 1.0f, 1.0f}, {u, v}, {0.0f, 1.0f, 0.0f});

				if (x < quads && z < quads)
				{
					const uint32_t topLeft = x * vertsPerAxis + z;
					const uint32_t topRight = (x + 1) * vertsPerAxis + z;
					const uint32_t bottomLeft = x * vertsPerAxis + (z + 1);
					const uint32_t bottomRight = (x + 1) * vertsPerAxis + (z + 1);

					builder.AddIndices({topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight});
				}
			}
		}

		return builder.Build();
	}
}
