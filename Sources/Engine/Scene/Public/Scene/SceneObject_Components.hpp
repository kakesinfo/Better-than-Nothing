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
#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	class SceneObject;

	struct Component
	{};

	struct NameComponent : Component
	{
		std::string name;

		NameComponent() = default;
		explicit NameComponent(std::string_view n) : name(n) {}
	};

	struct HierarchyComponent : Component
	{
		entt::entity parent{entt::null};
		std::vector<entt::entity> children;
	};

	struct TransformComponent : Component
	{
		Matrix4 localTransform{Matrix4::Identity};
		Matrix4 worldTransform{Matrix4::Identity};
		bool	isDirty{true};
	};

	struct MaterialComponent : Component
	{
		ResourceId materialId{0};
	};

	struct MeshComponent : Component
	{
		ResourceId meshId{0};
		ResourceId materialId{0};
		ResourceId textureId{0}; // Kept for backward compatibility
	};

	struct ModelComponent : Component
	{
		std::string				 path;
		bool					 preTransform = false;
		std::vector<SceneObject> children;
	};

	struct UpdateComponent : Component
	{
		std::function<void(SceneObject&, float)> onUpdate;
	};
}
