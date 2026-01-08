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

#include "ResourceManager/ResourceId.hpp"
#include "ResourceManager/ResourceManager.hpp"

namespace BetterThanNothing
{
	class RenderList;
	class Matrix4;

	class RenderContext
	{
	public:
		explicit RenderContext(RenderList* renderList) : m_renderList(renderList) {}
		~RenderContext() = default;

		void SubmitModel(const std::string& modelPath, const Matrix4& transform, bool preTransform = true) const;
		void SubmitModel(const ModelResource* modelResource, const Matrix4& transform) const;
		void SubmitModel(ResourceId modelId, const Matrix4& transform) const;
		void SubmitMesh(const MeshData& mesh, const Matrix4& transform) const;
		void SubmitMesh(ResourceId meshId, ResourceId textureId, const Matrix4& transform) const;

	private:
		RenderList* m_renderList;
	};
}
