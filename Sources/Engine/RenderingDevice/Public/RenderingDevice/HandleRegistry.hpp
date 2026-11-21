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

		[[nodiscard]] bool			  Has(Handle handle) const;
		[[nodiscard]] dataType*		  Get(Handle handle);
		[[nodiscard]] const dataType* Get(Handle handle) const;
		void						  Destroy(std::uint32_t index);
		void						  Destroy(Handle handle);

		[[nodiscard]] std::uint32_t Size() const;
		[[nodiscard]] bool			IsActive(std::uint32_t index) const;
		[[nodiscard]] dataType*		Get(std::uint32_t index);

		template <typename Fn>
		void ForEachActive(Fn&& fn);

	private:
		static constexpr uint32_t MaxGeneration = 0x00FFFFFFu;

		static uint32_t NextGeneration(const uint32_t current) noexcept
		{
			const uint32_t next = (current + 1u) & MaxGeneration;
			return next == 0u ? 1u : next;
		}

		std::vector<HandleRegistrySlot<dataType>> m_slots;
		std::vector<std::uint32_t>				  m_freeIndices;
	};

	template <typename dataType, HandleType handleType>
	static uint32_t Internal_GetFreeIndex(std::vector<HandleRegistrySlot<dataType>>& slots, std::vector<std::uint32_t>& freeIndices)
	{
		if (!freeIndices.empty())
		{
			const uint32_t idx = freeIndices.back();
			freeIndices.pop_back();
			return idx;
		}

		const auto idx = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
		return idx;
	}

	template <typename dataType, HandleType handleType>
	static bool Internal_IsValidHandle(const std::vector<HandleRegistrySlot<dataType>>& slots, Handle handle)
	{
		if (!handle.IsValid())
			return false;

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
		const uint32_t idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);

		HandleRegistrySlot<dataType>& slot = m_slots[idx];
		slot.active = true;
		slot.data = dataType(std::forward<Args>(args)...);

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	Handle HandleRegistry<dataType, handleType>::Allocate()
	{
		const std::uint32_t			  idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);
		HandleRegistrySlot<dataType>& slot = m_slots[idx];

		slot.active = true;
		slot.data = dataType{};

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	Handle HandleRegistry<dataType, handleType>::Append(dataType&& data)
	{
		const std::uint32_t			  idx = Internal_GetFreeIndex<dataType, handleType>(m_slots, m_freeIndices);
		HandleRegistrySlot<dataType>& slot = m_slots[idx];

		slot.active = true;
		slot.data = std::move(data);

		return Handle(idx, slot.generation, static_cast<uint8_t>(handleType));
	}

	template <typename dataType, HandleType handleType>
	bool HandleRegistry<dataType, handleType>::Has(Handle handle) const
	{
		return Internal_IsValidHandle<dataType, handleType>(m_slots, handle);
	}

	template <typename dataType, HandleType handleType>
	dataType* HandleRegistry<dataType, handleType>::Get(Handle handle)
	{
		if (!Internal_IsValidHandle<dataType, handleType>(m_slots, handle))
			throw std::runtime_error("Invalid handle!");
		return &m_slots[handle.GetId()].data;
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
			throw std::out_of_range("Index out of range");

		HandleRegistrySlot<dataType>& slot = m_slots[index];
		if (!slot.active)
			return;

		slot.active = false;
		slot.data = dataType{};
		slot.generation = NextGeneration(slot.generation);

		m_freeIndices.push_back(index);
	}

	template <typename dataType, HandleType handleType>
	void HandleRegistry<dataType, handleType>::Destroy(Handle handle)
	{
		if (!Internal_IsValidHandle<dataType, handleType>(m_slots, handle))
			throw std::runtime_error("Invalid handle!");

		Destroy(handle.GetId());
	}

	template <typename dataType, HandleType handleType>
	std::uint32_t HandleRegistry<dataType, handleType>::Size() const
	{
		return static_cast<std::uint32_t>(m_slots.size());
	}

	template <typename dataType, HandleType handleType>
	bool HandleRegistry<dataType, handleType>::IsActive(std::uint32_t index) const
	{
		return index < m_slots.size() && m_slots[index].active;
	}

	template <typename dataType, HandleType handleType>
	dataType* HandleRegistry<dataType, handleType>::Get(std::uint32_t index)
	{
		return &m_slots.at(index).data;
	}

	template <typename dataType, HandleType handleType>
	template <typename Fn>
	void HandleRegistry<dataType, handleType>::ForEachActive(Fn&& fn)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_slots.size()); ++i)
		{
			if (m_slots[i].active)
				fn(i, m_slots[i].data);
		}
	}
}
