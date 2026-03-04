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
	template <typename dataType>
	struct HandleRegistrySlot
	{
		dataType	  data;
		uint32_t generation = 1;
		bool		  active = false;
	};

	template <typename dataType, HandleType handleType>
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
		Handle Create(Args&&... args);
		Handle Allocate();
		Handle Append(dataType&& data);

		bool	 Has(Handle handle);
		dataType*		 Get(Handle handle);
		const dataType* Get(Handle handle) const;
		void	 Destroy(std::uint32_t index);
		void	 Destroy(Handle handle);

		std::uint32_t Size();
		dataType*			  Get(std::uint32_t index);

	private:
		std::vector<HandleRegistrySlot<dataType>> m_slots;
		std::vector<std::uint32_t>			 m_freeIndices;
	};

	template <typename dataType, HandleType handleType>
	static uint32_t Internal_GetFreeIndex(std::vector<HandleRegistrySlot<dataType>>& slots, std::vector<std::uint32_t>& freeIndices)
	{
		uint32_t idx;

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

	template <typename dataType, HandleType handleType>
	static bool Internal_IsValidHandle(std::vector<HandleRegistrySlot<dataType>>& slots, Handle handle)
	{
		if (handle.GetType() != static_cast<uint8_t>(handleType))
			return false;

		if (handle.GetId() >= slots.size())
			return false;

		const HandleRegistrySlot<dataType>& slot = slots[handle.GetId()];

		if (!slot.active)
			return false;
		if (slot.generation != handle.GetGeneration())
			return false;

		return true;
	}

	template <typename dataType, HandleType handleType>
	template <typename... Args>
	Handle HandleRegistry<dataType, handleType>::Create(Args&&... args)
	{
		uint32_t idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);

		HandleRegistrySlot<dataType>& slot = m_slots[idx];
		slot.active = true;
		slot.data = T(std::forward<Args>(args)...);

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	Handle HandleRegistry<dataType, handleType>::Allocate()
	{
		std::uint32_t			 idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);
		HandleRegistrySlot<dataType>& slot = m_slots[idx];

		slot.active = true;

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	Handle HandleRegistry<dataType, handleType>::Append(dataType&& data)
	{
		std::uint32_t			 idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);
		HandleRegistrySlot<dataType>& slot = m_slots[idx];

		slot.active = true;
		slot.data = std::move(data);

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	bool HandleRegistry<dataType, handleType>::Has(Handle handle)
	{
		return Internal_IsValidHandle<dataType, handleType>(m_slots, handle);
	}

	template <typename dataType, HandleType handleType>
	dataType* HandleRegistry<dataType, handleType>::Get(Handle handle)
	{
		if (!Internal_IsValidHandle<dataType, handleType>(m_slots, handle))
			throw std::runtime_error("Invalid handle!");
		return &m_slots.at(handle.GetId()).data;
	}

	template <typename dataType, HandleType handleType>
	const dataType* HandleRegistry<dataType, handleType>::Get(Handle handle) const
	{
		if (!Internal_IsValidHandle<dataType, handleType>(m_slots, handle))
			throw std::runtime_error("Invalid handle!");
		return &m_slots[handle.GetId()].data;
	}

	template <typename dataType, HandleType handleType>
	void HandleRegistry<dataType, handleType>::Destroy(std::uint32_t index)
	{
		if (index >= m_slots.size())
			throw std::overflow_error("Index out of range");

		HandleRegistrySlot<dataType>& slot = m_slots.at(index);

		slot.active = false;
		slot.data = dataType{};
		++slot.generation;

		// TODO: Prevent m_generation to overflow

		m_freeIndices.push_back(index);
	}

	template <typename dataType, HandleType handleType>
	void HandleRegistry<dataType, handleType>::Destroy(Handle handle)
	{
		if (!Internal_IsValidHandle<dataType, handleType>(m_slots, handle))
			throw std::runtime_error("Invalid handle!");

		HandleRegistrySlot<dataType>& slot = m_slots.at(handle.GetId());

		slot.active = false;
		slot.data = dataType{};
		++slot.generation;

		// TODO: Prevent m_generation to overflow

		m_freeIndices.push_back(handle.GetId());
	}

	template <typename dataType, HandleType handleType>
	std::uint32_t HandleRegistry<dataType, handleType>::Size()
	{
		return m_slots.size();
	}

	template <typename dataType, HandleType handleType>
	dataType* HandleRegistry<dataType, handleType>::Get(std::uint32_t index)
	{
		return &m_slots.at(index).data;
	}
}
