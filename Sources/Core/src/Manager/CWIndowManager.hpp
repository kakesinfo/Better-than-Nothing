//
// Created by acoezard on 28/11/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CWindowManagerError : public std::runtime_error
	{
	public:
		explicit CWindowManagerError(const std::string& message)
			: std::runtime_error(message) {}
	};

	struct WindowManagerParams
	{
		std::string title;
		uint32 width = 0;
		uint32 height = 0;
		bool fullscreen = false;
		bool resizable = false;
	};

	class CWindowManager
	{
	private:
		static std::unique_ptr<CWindow> m_window;

	public:
		CWindowManager() = default;
		~CWindowManager() = default;

		static void Initialize(const WindowManagerParams& params)
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

		static std::unique_ptr<CWindow>& GetWindow()
		{
			if (m_window == nullptr)
				throw CWindowManagerError("WindowManager is not initialized.");
			return m_window;
		}

		static void Shutdown()
		{
			m_window.reset();
		}
	};
};
