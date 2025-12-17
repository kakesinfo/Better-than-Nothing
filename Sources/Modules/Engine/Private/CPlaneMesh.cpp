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