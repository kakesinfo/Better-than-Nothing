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

#include "Module_Engine/CRenderContext.hpp"
#include "Module_Renderer/CRenderList.hpp"

namespace BetterThanNothing
{
	void CRenderContext::Submit(const ModelData* model, const glm::mat4& transform) const
	{
		m_renderList->Submit(model, transform);
	}

	void CRenderContext::Submit(const ModelData* model, const glm::mat4& transform, const std::string& texturePath) const
	{
		m_renderList->Submit(model, transform, texturePath);
	}
}