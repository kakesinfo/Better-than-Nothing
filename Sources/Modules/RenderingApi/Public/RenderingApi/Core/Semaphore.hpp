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

namespace BetterThanNothing::RenderingApi
{
	class Device;

	struct SemaphoreCreateInfo
	{
		Device* device;
	};

	class Semaphore
	{
	public:
		explicit Semaphore(const SemaphoreCreateInfo& createInfo);
		~Semaphore();

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore(Semaphore&& other) = delete;
		Semaphore& operator=(Semaphore&& other) = delete;

		static std::unique_ptr<Semaphore> Create(const SemaphoreCreateInfo& createInfo);

		[[nodiscard]] VkSemaphore Handle() const;

	private:
		Device*		m_device;
		VkSemaphore m_semaphore = VK_NULL_HANDLE;
	};
}
