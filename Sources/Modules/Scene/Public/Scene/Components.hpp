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

#include "Module_Math/Matrix4.hpp"

namespace BetterThanNothing
{
	enum class ComponentType
	{
		Transform = 0,
		MAX_COMPONENTS
	};

	struct Component
	{};

	struct TransformComponent : Component
	{
		Matrix4	   localMatrix{};
		Matrix4	   worldMatrix{};
		bool	   dirty = false;
	};

	using ComponentArray = std::array<Component, static_cast<uint32_t>(ComponentType::MAX_COMPONENTS)>;
}
