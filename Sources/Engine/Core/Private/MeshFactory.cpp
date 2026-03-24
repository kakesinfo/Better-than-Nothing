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
	MeshData MeshFactory::CreatePlane(const float size)
	{
		const auto	subdivisions = static_cast<int32_t>(size);
		const float halfSize = size / 2.0f;

		MeshBuilder builder;
		for (int32_t x = 0; x < subdivisions; x++)
		{
			for (int32_t z = 0; z < subdivisions; z++)
			{
				const float posX = (static_cast<float>(x) / size) * size - halfSize;
				const float posZ = (static_cast<float>(z) / size) * size - halfSize;

				builder.AddVertex({posX, 0.0f, posZ}, {1.0f, 1.0f, 1.0f},
								  {static_cast<float>(x), static_cast<float>(z)}, {0.0f, 1.0f, 0.0f});

				if (x < subdivisions - 1 && z < subdivisions - 1)
				{
					const uint32_t topLeft = x * subdivisions + z;
					const uint32_t topRight = (x + 1) * subdivisions + z;
					const uint32_t bottomLeft = x * subdivisions + (z + 1);
					const uint32_t bottomRight = (x + 1) * subdivisions + (z + 1);

					builder.AddIndices({topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight});
				}
			}
		}

		return builder.Build();
	}
}
