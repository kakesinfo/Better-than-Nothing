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
			const glm::mat4& m = viewProjection.Value();
			const glm::vec4	 row0(m[0][0], m[1][0], m[2][0], m[3][0]);
			const glm::vec4	 row1(m[0][1], m[1][1], m[2][1], m[3][1]);
			const glm::vec4	 row2(m[0][2], m[1][2], m[2][2], m[3][2]);
			const glm::vec4	 row3(m[0][3], m[1][3], m[2][3], m[3][3]);

			auto makePlane = [](const glm::vec4& eq) -> Plane {
				const float len = glm::length(glm::vec3(eq));
				if (len <= 0.0f)
					return {};
				return {Vector3(eq.x / len, eq.y / len, eq.z / len), eq.w / len};
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
			const glm::mat4& m = worldMatrix.Value();
			const glm::vec3	 localCenter = (localMin.Value() + localMax.Value()) * 0.5f;
			const glm::vec3	 localExtents = (localMax.Value() - localMin.Value()) * 0.5f;

			const glm::vec3 worldCenter = glm::vec3(m * glm::vec4(localCenter, 1.0f));
			const glm::vec3 right(std::fabs(m[0][0]), std::fabs(m[0][1]), std::fabs(m[0][2]));
			const glm::vec3 up(std::fabs(m[1][0]), std::fabs(m[1][1]), std::fabs(m[1][2]));
			const glm::vec3 forward(std::fabs(m[2][0]), std::fabs(m[2][1]), std::fabs(m[2][2]));

			const glm::vec3 worldExtents = right * localExtents.x + up * localExtents.y + forward * localExtents.z;

			return IntersectsAABB(Vector3(worldCenter - worldExtents), Vector3(worldCenter + worldExtents));
		}

	private:
		std::array<Plane, 6> m_planes{};
	};
}
