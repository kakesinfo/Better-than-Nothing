#pragma once

#include "IEngineApplication.hpp"

namespace BetterThanNothing
{
	extern int BootEngine(IEngineApplication* app);
}

int main()
{
	const auto app = BetterThanNothing::CreateEngineApplication();
	return BetterThanNothing::BootEngine(app.get());
}