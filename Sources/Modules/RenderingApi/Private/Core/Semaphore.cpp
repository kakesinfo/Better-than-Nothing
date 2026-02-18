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

#include "RenderingApi.hpp"

namespace BetterThanNothing
{
	Semaphore::Semaphore(const SemaphoreCreateInfo& createInfo): m_device(createInfo.device)
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(m_device->Handle(), &semaphoreInfo, nullptr, &m_semaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create semaphore!");
	}

	Semaphore::~Semaphore()
	{
		if (m_semaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(m_device->Handle(), m_semaphore, nullptr);
	}

	std::unique_ptr<Semaphore> Semaphore::Create(const SemaphoreCreateInfo& createInfo)
	{
		return std::make_unique<Semaphore>(createInfo);
	}

	VkSemaphore Semaphore::Handle() const
	{
		return m_semaphore;
	}
}
