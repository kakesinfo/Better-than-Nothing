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

#pragma once

#include <functional>
#include <vector>

namespace BetterThanNothing
{
	class DeletionQueue
	{
	public:
		void Push(std::function<void()>&& deletor)
		{
			m_deletors.push_back(std::move(deletor));
		}

		void Flush()
		{
			for (auto it = m_deletors.rbegin(); it != m_deletors.rend(); ++it)
				(*it)();
			m_deletors.clear();
		}

		[[nodiscard]] bool Empty() const { return m_deletors.empty(); }

	private:
		std::vector<std::function<void()>> m_deletors;
	};
}
