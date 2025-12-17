//
// Created by acoezard on 28/11/2025.
//

#pragma once

namespace BetterThanNothing
{
	class CWindow;

	class CWindowManagerError final : public std::runtime_error
	{
	public:
		explicit CWindowManagerError(const std::string& message)
			: std::runtime_error(message) {}
	};

	struct WindowManagerParams
	{
		std::string		title;
		std::uint32_t	width = 0;
		std::uint32_t	height = 0;
		bool			fullscreen = false;
		bool			resizable = false;
	};

	class CWindowManager
	{
	public:
		CWindowManager() = default;
		~CWindowManager() = default;

		static void Initialize(const WindowManagerParams& params);
		static std::unique_ptr<CWindow>& GetWindow();
		static void Shutdown();
	private:
		static std::unique_ptr<CWindow> m_window;
	};
};
