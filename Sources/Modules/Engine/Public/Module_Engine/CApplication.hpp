#pragma once

namespace BetterThanNothing
{
	class CUpdateContext;
	class CRenderContext;

	class CApplication
	{
	public:
		CApplication() = default;
		virtual ~CApplication() = default;

		void Run();

		virtual void OnEnable() = 0;
		virtual void OnUpdate(const CUpdateContext* ctx) = 0;
		virtual void OnRender(const CRenderContext* ctx) = 0;
		virtual void OnDisable() = 0;
	};
};
