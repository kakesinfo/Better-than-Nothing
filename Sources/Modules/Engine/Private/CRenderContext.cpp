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