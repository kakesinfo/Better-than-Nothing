#pragma once

namespace BetterThanNothing
{
	class CRenderList;
	struct ModelData;

	class CRenderContext
	{
	public:
		explicit CRenderContext(CRenderList* renderList): m_renderList(renderList) {}
		~CRenderContext() = default;

		void Submit(const ModelData* model, const glm::mat4& transform) const;
		void Submit(const ModelData* model, const glm::mat4& transform, const std::string& texturePath) const;
	private:
		CRenderList* m_renderList;
	};
}
