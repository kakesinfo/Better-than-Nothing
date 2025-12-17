#pragma once

#include "Module_Engine/IEngineApplication.hpp"

namespace BetterThanNothing
{
	class CUpdateContext;
	class CRenderContext;

	class CRuntime final : public IEngineApplication {
	public:
		CRuntime() = default;

		void OnEnable() override;
		void OnUpdate(const CUpdateContext* ctx) override;
		void OnRender(const CRenderContext* ctx) override;
		void OnDisable() override;
	};
};
