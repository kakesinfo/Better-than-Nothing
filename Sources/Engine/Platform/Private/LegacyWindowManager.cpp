/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "../Public/Platform/LegacyWindowManager.hpp"
#include "Platform/LegacyWindow.hpp"

namespace BetterThanNothing
{
	std::unique_ptr<LegacyWindow> LegacyWindowManager::m_window = nullptr;

	void LegacyWindowManager::Initialize(const LegacyWindowManagerParams& params)
	{
		if (m_window != nullptr)
			throw LegacyWindowManagerError("LegacyWindowManager is already initialized.");

		m_window = std::make_unique<LegacyWindow>(params.title, params.width, params.height, params.fullscreen, params.resizable);
	}

	std::unique_ptr<LegacyWindow>& LegacyWindowManager::GetWindow()
	{
		if (m_window == nullptr)
			throw LegacyWindowManagerError("LegacyWindowManager is not initialized.");
		return m_window;
	}

	void LegacyWindowManager::Shutdown()
	{
		m_window.reset();
	}
};
