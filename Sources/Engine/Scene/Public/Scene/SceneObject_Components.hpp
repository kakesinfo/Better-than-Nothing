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

#include <Common/ResourceId.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Quaternion.hpp>
#include <Math/Vector3.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <string>

namespace BetterThanNothing
{
	class SceneObject;

	struct Component
	{};

	struct NameComponent : Component
	{
		std::string name;
	};

	struct HierarchyComponent : Component
	{
		entt::entity parent = entt::null;
		entt::entity firstChild = entt::null;
		entt::entity prevSibling = entt::null;
		entt::entity nextSibling = entt::null;
		uint32_t	 childCount = 0;
	};

	struct TransformComponent : Component
	{
		Vector3	   position = Vector3::Zero;
		Quaternion rotation = Quaternion::Identity;
		Vector3	   scale = Vector3::One;

		Matrix4 localMatrix = Matrix4::Identity;
		Matrix4 worldMatrix = Matrix4::Identity;
		Matrix4 prevWorldMatrix = Matrix4::Identity;
		Matrix4 inverseWorldMatrix = Matrix4::Identity;

		bool dirty = true;

		void SetPosition(const Vector3& pos)
		{
			position = pos;
			dirty = true;
		}

		void SetPosition(const float x, const float y, const float z)
		{
			position = Vector3(x, y, z);
			dirty = true;
		}

		void Translate(const Vector3& delta)
		{
			position += delta;
			dirty = true;
		}

		void SetRotation(const Quaternion& rot)
		{
			rotation = rot.Normalized();
			dirty = true;
		}

		void SetEulerDegrees(const Vector3& eulerDegrees)
		{
			rotation = Quaternion::FromEulerDegrees(eulerDegrees).Normalized();
			dirty = true;
		}

		void Rotate(const float angleDegrees, const Vector3& axis)
		{
			rotation = (Quaternion::AngleAxis(angleDegrees, axis) * rotation).Normalized();
			dirty = true;
		}

		void SetScale(const Vector3& s)
		{
			scale = s;
			dirty = true;
		}

		void SetScale(const float uniformScale)
		{
			scale = Vector3(uniformScale);
			dirty = true;
		}

		void SetLocalMatrix(const Matrix4& mat)
		{
			mat.Decompose(position, rotation, scale);
			localMatrix = mat;
			dirty = true;
		}

		void MarkDirty() { dirty = true; }

		[[nodiscard]] Vector3 GetWorldPosition() const { return worldMatrix.GetTranslation(); }
	};

	struct ModelComponent : Component
	{
		ResourceId	modelId = InvalidResourceId;
		std::string path;
		bool		preTransform = false;
	};

	struct MeshComponent : Component
	{
		ResourceId meshId = InvalidResourceId;
		ResourceId materialId = InvalidResourceId;
		ResourceId textureId = InvalidResourceId;
	};

	struct DirectionalLightComponent : Component
	{
		Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
		Vector3 color = Vector3::One;
		float	illuminanceLux = 100000.0f;
	};

	struct PointLightComponent : Component
	{
		Vector3 color = Vector3::One;
		float	luminousPowerLm = 150000.0f;
		float	radius = 25.0f;
	};

	struct UpdateComponent : Component
	{
		std::function<void(SceneObject&, float)> onUpdate;
	};

	struct GrassChunkComponent : Component
	{
		int32_t chunkX = 0;
		int32_t chunkZ = 0;
	};

	struct WaterChunkComponent : Component
	{
		int32_t chunkX = 0;
		int32_t chunkZ = 0;
	};
}
