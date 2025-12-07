#pragma once

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	class CRuntime: public CApplication {
	public:
		CRuntime() = default;

		void OnEnable(IRenderer* renderer) override;
		void OnUpdate() override;
		void OnRender(CRenderQueue* renderQueue) override;
		void OnDisable() override;

	private:
		CMesh mesh;
	};
};
