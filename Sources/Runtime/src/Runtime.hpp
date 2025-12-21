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

#include "Module_Engine/IEngineApplication.hpp"
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Math/Matrix4.hpp>

namespace BetterThanNothing
{
	class UpdateContext;
	class RenderContext;

	class Matrix4;

	class Runtime final : public IEngineApplication
	{
	public:
		Runtime() = default;

		void OnEnable() override;
		void OnUpdate(const UpdateContext* ctx) override;
		void OnRender(const RenderContext* ctx) override;
		void OnDisable() override;

		void SyncModels();

	private:
		std::future<ModelData>	 m_sponzaFuture;
		std::optional<ModelData> m_sponzaModel;
		Matrix4					 m_sponzaTransform;

		std::future<ModelData>	 m_robotFuture;
		std::optional<ModelData> m_robotModel;
		Matrix4					 m_robotTransform;
	};
};
