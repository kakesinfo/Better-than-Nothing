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
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "Matrix4.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include <array>
#include <cmath>

namespace BetterThanNothing
{
	struct Plane
	{
		Vector3 normal = Vector3::Up;
		float	distance = 0.0f;

		[[nodiscard]] float SignedDistance(const Vector3& point) const
		{
			return Vector3::Dot(normal, point) + distance;
		}
	};

	class Frustum
	{
	public:
		// Extracts 6 normalized frustum planes from a Vulkan clip-space (z in [0, 1]) ViewProjection matrix.
		[[nodiscard]] static Frustum FromViewProjection(const Matrix4& viewProjection)
		{
			const Vector4 row0 = viewProjection.GetRow(0);
			const Vector4 row1 = viewProjection.GetRow(1);
			const Vector4 row2 = viewProjection.GetRow(2);
			const Vector4 row3 = viewProjection.GetRow(3);

			auto makePlane = [](const Vector4& eq) -> Plane {
				const Vector3 n(eq.GetX(), eq.GetY(), eq.GetZ());
				const float	  len = n.Magnitude();
				if (len <= 0.0f)
					return {};
				return {n / len, eq.GetW() / len};
			};

			Frustum f;
			f.m_planes[0] = makePlane(row3 + row0); // Left
			f.m_planes[1] = makePlane(row3 - row0); // Right
			f.m_planes[2] = makePlane(row3 + row1); // Bottom
			f.m_planes[3] = makePlane(row3 - row1); // Top
			f.m_planes[4] = makePlane(row2);		// Near (Vulkan z >= 0)
			f.m_planes[5] = makePlane(row3 - row2); // Far  (Vulkan z <= w)
			return f;
		}

		[[nodiscard]] bool IntersectsAABB(const Vector3& minBounds, const Vector3& maxBounds) const
		{
			const Vector3 center = (minBounds + maxBounds) * 0.5f;
			const Vector3 extents = (maxBounds - minBounds) * 0.5f;

			for (const Plane& plane : m_planes)
			{
				const float r = extents.GetX() * std::fabs(plane.normal.GetX()) +
								extents.GetY() * std::fabs(plane.normal.GetY()) +
								extents.GetZ() * std::fabs(plane.normal.GetZ());

				if (plane.SignedDistance(center) + r < 0.0f)
					return false;
			}
			return true;
		}

		[[nodiscard]] bool IntersectsLocalAABB(const Vector3& localMin, const Vector3& localMax, const Matrix4& worldMatrix) const
		{
			const Vector3 localCenter = (localMin + localMax) * 0.5f;
			const Vector3 localExtents = (localMax - localMin) * 0.5f;

			const Vector3 worldCenter = worldMatrix.TransformPoint(localCenter);

			const Vector4 col0 = worldMatrix.GetColumn(0);
			const Vector4 col1 = worldMatrix.GetColumn(1);
			const Vector4 col2 = worldMatrix.GetColumn(2);

			const Vector3 right(std::fabs(col0.GetX()), std::fabs(col0.GetY()), std::fabs(col0.GetZ()));
			const Vector3 up(std::fabs(col1.GetX()), std::fabs(col1.GetY()), std::fabs(col1.GetZ()));
			const Vector3 forward(std::fabs(col2.GetX()), std::fabs(col2.GetY()), std::fabs(col2.GetZ()));

			const Vector3 worldExtents = right * localExtents.GetX() + up * localExtents.GetY() + forward * localExtents.GetZ();

			return IntersectsAABB(worldCenter - worldExtents, worldCenter + worldExtents);
		}

	private:
		std::array<Plane, 6> m_planes{};
	};
}
