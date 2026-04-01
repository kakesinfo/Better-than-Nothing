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
#include <string>
#include <vector>

namespace BetterThanNothing
{
	class PipelineBlueprint
	{
	public:
		explicit PipelineBlueprint() = default;
		~PipelineBlueprint() = default;

		PipelineBlueprint* AddShader(ResourceId id);
		PipelineBlueprint* SetPipelineLayout(const std::string& name);

		[[nodiscard]] std::vector<ResourceId> GetShaders() const;
		[[nodiscard]] std::string			  GetPipelineLayout() const;

	private:
		std::vector<ResourceId> m_shaders;
		std::string				m_pipelineLayout;
	};
}
