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

namespace BetterThanNothing
{
	struct ModelData;
	struct MeshGpuData;
	struct TextureGpuData;
	struct MeshData;

	class RenderCache;

	struct RenderListItem
	{
		MeshGpuData*	meshGpuData;
		TextureGpuData*	textureGpuData;
		glm::mat4		transform;
	};

	class RenderList
	{
	public:
		explicit RenderList(RenderCache* renderCache): m_renderCache(renderCache) {}
		~RenderList() = default;

		void Submit(const ModelData* model, const glm::mat4& transform, const std::string& texturePath);
		void Submit(const ModelData* model, const glm::mat4& transform);
		void Submit(const MeshData* mesh, const glm::mat4& transform);
		void Submit(const RenderListItem& item);

		[[nodiscard]]
		RenderListItem Get(std::uint32_t index) const;

		void Clear();

		[[nodiscard]] std::uint32_t Size() const;
		[[nodiscard]] bool IsEmpty() const;

	private:
		RenderCache* 				m_renderCache;
		std::vector<RenderListItem>	m_list;
	};
}
