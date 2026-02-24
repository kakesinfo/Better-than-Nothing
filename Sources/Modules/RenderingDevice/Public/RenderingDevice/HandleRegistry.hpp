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

#include "Handle.hpp"
#include <vector>

namespace BetterThanNothing
{
	template <typename T>
	struct HandleRegistrySlot
	{
		T			  data;
		std::uint32_t generation = 1;
		bool		  active = false;
	};

	template <typename T>
	class HandleRegistry
	{
	public:
		HandleRegistry() = default;
		~HandleRegistry() = default;

		HandleRegistry(const HandleRegistry&) = delete;
		HandleRegistry& operator=(const HandleRegistry&) = delete;
		HandleRegistry(HandleRegistry&&) = delete;
		HandleRegistry& operator=(HandleRegistry&&) = delete;

		template <typename... Args>
		Handle<T> Create(Args&&... args);
		Handle<T> Allocate();
		Handle<T> Append(T&& data);

		bool	 Has(Handle<T> handle);
		T*		 Get(Handle<T> handle);
		const T* Get(Handle<T> handle) const;
		void	 Destroy(std::uint32_t index);
		void	 Destroy(Handle<T> handle);

		std::uint32_t Size();
		T*			  Get(std::uint32_t index);

	private:
		std::vector<HandleRegistrySlot<T>> m_slots;
		std::vector<std::uint32_t>			 m_freeIndices;
	};

	template <typename T>
	static std::uint32_t GetFreeIndex_Internal(std::vector<HandleRegistrySlot<T>>& slots, std::vector<std::uint32_t>& freeIndices)
	{
		std::uint32_t idx;

		if (!freeIndices.empty())
		{
			idx = freeIndices.back();
			freeIndices.pop_back();
			return idx;
		}

		idx = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
		return idx;
	}

	template <typename T>
	bool IsValidHandle_Internal(std::vector<HandleRegistrySlot<T>>& slots, Handle<T> handle)
	{
		if (handle.GetId() >= slots.size())
			return false;

		const HandleRegistrySlot<T>& slot = slots[handle.GetId()];

		if (!slot.active)
			return false;
		if (slot.generation != handle.GetGeneration())
			return false;

		return true;
	}

	template <typename T>
	template <typename... Args>
	Handle<T> HandleRegistry<T>::Create(Args&&... args)
	{
		uint32_t idx = GetFreeIndex_Internal(m_slots, m_freeIndices);

		HandleRegistrySlot<T>& slot = m_slots[idx];
		slot.active = true;
		slot.data = T(std::forward<Args>(args)...);

		return Handle<T>(idx, slot.generation);
	}

	template <typename T>
	Handle<T> HandleRegistry<T>::Allocate()
	{
		std::uint32_t			 idx = GetFreeIndex_Internal(m_slots, m_freeIndices);
		HandleRegistrySlot<T>& slot = m_slots[idx];

		slot.active = true;

		return Handle<T>(idx, slot.generation);
	}

	template <typename T>
	Handle<T> HandleRegistry<T>::Append(T&& data)
	{
		std::uint32_t			 idx = GetFreeIndex_Internal(m_slots, m_freeIndices);
		HandleRegistrySlot<T>& slot = m_slots[idx];

		slot.active = true;
		slot.data = std::move(data);

		return Handle<T>(idx, slot.generation);
	}

	template <typename T>
	bool HandleRegistry<T>::Has(Handle<T> handle)
	{
		return IsValidHandle_Internal(m_slots, handle);
	}

	template <typename T>
	T* HandleRegistry<T>::Get(Handle<T> handle)
	{
		if (!IsValidHandle_Internal(m_slots, handle))
			return nullptr;
		return &m_slots.at(handle.GetId()).data;
	}

	template <typename T>
	const T* HandleRegistry<T>::Get(Handle<T> handle) const
	{
		if (!IsValidHandle_Internal(m_slots, handle))
			return nullptr;
		return &m_slots[handle.GetId()].data;
	}

	template <typename T>
	void HandleRegistry<T>::Destroy(std::uint32_t index)
	{
		if (index >= m_slots.size())
			throw std::overflow_error("Index out of range");

		HandleRegistrySlot<T>& slot = m_slots.at(index);

		slot.active = false;
		slot.data = T{};
		++slot.generation;

		// TODO: Prevent m_generation to overflow

		m_freeIndices.push_back(index);
	}

	template <typename T>
	void HandleRegistry<T>::Destroy(Handle<T> handle)
	{
		if (!IsValidHandle_Internal(m_slots, handle))
			return;

		HandleRegistrySlot<T>& slot = m_slots.at(handle.GetId());

		slot.active = false;
		slot.data = T{};
		++slot.generation;

		// TODO: Prevent m_generation to overflow

		m_freeIndices.push_back(handle.GetId());
	}

	template <typename T>
	std::uint32_t HandleRegistry<T>::Size()
	{
		return m_slots.size();
	}

	template <typename T>
	T* HandleRegistry<T>::Get(std::uint32_t index)
	{
		return &m_slots.at(index).data;
	}
}
