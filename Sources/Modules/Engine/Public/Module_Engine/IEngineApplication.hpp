/*
*	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include <memory>

namespace BetterThanNothing
{
	class CUpdateContext;
	class CRenderContext;

	class IEngineApplication
	{
	public:
		virtual ~IEngineApplication() = default;

		virtual void OnEnable() = 0;
		virtual void OnUpdate(const CUpdateContext* ctx) = 0;
		virtual void OnRender(const CRenderContext* ctx) = 0;
		virtual void OnDisable() = 0;
	};

	extern std::unique_ptr<IEngineApplication> CreateEngineApplication();
}
