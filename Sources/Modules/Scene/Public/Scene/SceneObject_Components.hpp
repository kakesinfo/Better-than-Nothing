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

namespace BetterThanNothing
{
	class SceneObject;

	struct Component
	{};

	struct NameComponent : Component
	{
		std::string name;
	};

	struct TransformComponent : Component
	{
		Matrix4 localTransform;
		// TODO: add global and inverse matrices
		// TODO: add a dirty flag
	};

	struct ModelComponent : Component
	{
		std::string				 path;
		bool					 preTransform = false;
		std::vector<SceneObject> children;
	};

	struct MeshComponent : Component
	{
		ResourceId meshId;
		ResourceId textureId;
	};

	struct UpdateComponent : Component
	{
		std::function<void(SceneObject&, float)> onUpdate;
	};
}
