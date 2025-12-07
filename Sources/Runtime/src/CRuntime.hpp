#pragma once

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	class CRuntime final : public CApplication {
	public:
		CRuntime() = default;

		void OnEnable(IRenderer* renderer) override;
		void OnUpdate() override;
		void OnRender(CRenderList* renderList) override;
		void OnDisable() override;
	};
};
