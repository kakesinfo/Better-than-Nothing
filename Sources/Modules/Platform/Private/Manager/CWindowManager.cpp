#include "module_platform.hpp"

namespace BetterThanNothing
{
	std::unique_ptr<CWindow> CWindowManager::m_window = nullptr;

	void CWindowManager::Initialize(const WindowManagerParams& params)
	{
		if (m_window != nullptr)
			throw CWindowManagerError("WindowManager is already initialized.");

		m_window = std::make_unique<CWindow>(
			params.title,
			params.width,
			params.height,
			params.fullscreen,
			params.resizable
		);
	}

	std::unique_ptr<CWindow>& CWindowManager::GetWindow()
	{
		if (m_window == nullptr)
			throw CWindowManagerError("WindowManager is not initialized.");
		return m_window;
	}

	void CWindowManager::Shutdown()
	{
		m_window.reset();
	}
};
