#pragma once

#include "CWindow.hpp"

namespace BetterThanNothing
{
	class CApplicationContext;

	class CApplication
	{
	protected:
		CWindow m_window;

	public:

		CApplication() = default;
		virtual ~CApplication() = default;

		void Run();

		virtual void OnEnable(CApplicationContext* context) = 0;
		virtual void OnUpdate(CApplicationContext* context) = 0;
		virtual void OnRender(CApplicationContext* context) = 0;
		virtual void OnDisable(CApplicationContext* context) = 0;
	};
};
