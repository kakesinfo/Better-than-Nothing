/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Module_Platform/Manager/CWindowManager.hpp"
#include "Module_Platform/CWindow.hpp"

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
