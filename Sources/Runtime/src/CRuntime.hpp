#pragma once

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	class CApplicationContext;

	class CRuntime: public CApplication {
	public:
		CRuntime() = default;

		void OnEnable(CApplicationContext* context) override;
		void OnUpdate(CApplicationContext* context) override;
		void OnRender(CApplicationContext* context) override;
		void OnDisable(CApplicationContext* context) override;
	};
};
