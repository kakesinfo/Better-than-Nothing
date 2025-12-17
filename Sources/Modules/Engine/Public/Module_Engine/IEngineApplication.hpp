#pragma once

#include <memory>

namespace BetterThanNothing
{
	class CUpdateContext;
	class CRenderContext;

	class IEngineApplication
	{
	public:
		virtual ~IEngineApplication() = default;

		virtual void OnEnable() = 0;
		virtual void OnUpdate(const CUpdateContext* ctx) = 0;
		virtual void OnRender(const CRenderContext* ctx) = 0;
		virtual void OnDisable() = 0;
	};

	extern std::unique_ptr<IEngineApplication> CreateEngineApplication();
}
