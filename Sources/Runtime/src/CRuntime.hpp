#pragma once

#include "Module_Engine/CApplication.hpp"

namespace BetterThanNothing
{
	class CUpdateContext;
	class CRenderContext;

	class CRuntime final : public CApplication {
	public:
		CRuntime() = default;

		void OnEnable() override;
		void OnUpdate(const CUpdateContext* ctx) override;
		void OnRender(const CRenderContext* ctx) override;
		void OnDisable() override;
	};
};
