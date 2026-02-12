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

#include "Module_Engine/RenderContext.hpp"
#include "Renderer/RenderList.hpp"

namespace BetterThanNothing
{
	void RenderContext::SubmitModel(const std::string& modelPath, const Matrix4& transform, bool preTransform) const
	{
		m_renderList->SubmitModel(modelPath, transform, preTransform);
	}

	void RenderContext::SubmitModel(const ResourceId modelId, const Matrix4& transform) const
	{
		m_renderList->SubmitModel(modelId, transform);
	}

	void RenderContext::SubmitModel(const ModelResource* modelResource, const Matrix4& transform) const
	{
		m_renderList->SubmitModel(modelResource, transform);
	}

	void RenderContext::SubmitMesh(const MeshData& mesh, const Matrix4& transform) const
	{
		m_renderList->SubmitMesh(mesh, transform);
	}

	void RenderContext::SubmitMesh(ResourceId meshId, ResourceId textureId, const Matrix4& transform) const
	{
		m_renderList->SubmitMesh(meshId, textureId, transform);
	}
}
