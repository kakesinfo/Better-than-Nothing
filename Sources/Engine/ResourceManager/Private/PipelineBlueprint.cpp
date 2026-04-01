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

#include <ResourceManager/PipelineBlueprint.hpp>

namespace BetterThanNothing
{
	PipelineBlueprint* PipelineBlueprint::AddShader(ResourceId id)
	{
		m_shaders.push_back(id);
		return this;
	}

	PipelineBlueprint* PipelineBlueprint::SetPipelineLayout(const std::string& name)
	{
		m_pipelineLayout = name;
		return this;
	}

	std::vector<ResourceId> PipelineBlueprint::GetShaders() const
	{
		return m_shaders;
	}

	std::string PipelineBlueprint::GetPipelineLayout() const
	{
		return m_pipelineLayout;
	}
}
