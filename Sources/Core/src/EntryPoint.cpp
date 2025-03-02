#include "CApplication.hpp"

using namespace BetterThanNothing;

extern CApplication* CreateApplication();

int main() {
	auto pApplication = CreateApplication();

	try {
		pApplication->Run();
	} catch (const std::exception& e) {
		LOG_ERROR(e.what());
	}

	delete pApplication;

	return EXIT_SUCCESS;
}
