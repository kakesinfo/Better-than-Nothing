#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CPlaneMesh::CPlaneMesh(const float32 size)
	{
		_generatePlane(size);
	}

	void CPlaneMesh::_generatePlane(const float32 size)
	{
		const int32 subdivisions = static_cast<int32>(size);
		const float halfSize = size / 2.0f;

		for (int32 x = 0; x < subdivisions; x++)
		{
			for (int32 z = 0; z < subdivisions; z++)
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
					const uint32 topLeft = x * subdivisions + z;
					const uint32 topRight = (x + 1) * subdivisions + z;
					const uint32 bottomLeft = x * subdivisions + (z + 1);
					const uint32 bottomRight = (x + 1) * subdivisions + (z + 1);

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