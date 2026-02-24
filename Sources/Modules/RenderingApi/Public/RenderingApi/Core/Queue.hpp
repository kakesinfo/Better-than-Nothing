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
	struct QueueCreateInfo
	{
		VkQueue	 handle = VK_NULL_HANDLE;
		uint32_t familyIndex = 0;
		float	 priority = 1.0f;
	};

	class Queue
	{
	public:
		explicit Queue(const QueueCreateInfo& createInfo);
		~Queue();

		Queue(const Queue&) = delete;
		Queue& operator=(const Queue&) = delete;
		Queue(Queue&&) = delete;
		Queue& operator=(Queue&&) = delete;

		static std::unique_ptr<Queue> Create(const QueueCreateInfo& createInfo);

		VkResult Submit(uint32_t submitCount, const VkSubmitInfo* submits, VkFence fence) const;
		VkResult Present(const VkPresentInfoKHR* presentInfo) const;
		VkResult WaitIdle() const;

		[[nodiscard]] uint32_t GetFamilyIndex() const;
		[[nodiscard]] float	   GetPriority() const;
		[[nodiscard]] VkQueue  Handle() const;

	private:
		QueueCreateInfo	   m_createInfo;
		mutable std::mutex m_mutex{};
	};
}
