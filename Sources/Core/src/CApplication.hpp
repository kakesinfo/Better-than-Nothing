#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CApplication
	{
	protected:
		CWindow m_window;

	public:

		CApplication() = default;
		virtual ~CApplication() = default;

		void Run();

		virtual void OnEnable(IRenderer* renderer) = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender(CRenderQueue* renderQueue) = 0;
		virtual void OnDisable() = 0;
	};
};
