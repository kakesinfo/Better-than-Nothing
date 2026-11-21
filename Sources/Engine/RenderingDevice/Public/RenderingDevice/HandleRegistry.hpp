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
#include <Common/Containers/SlotMap.hpp>
#include <stdexcept>
#include <utility>

namespace BetterThanNothing
{
	template <typename DataType, HandleType Type>
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
		Handle Create(Args&&... args)
		{
			const auto key = m_storage.Emplace(std::forward<Args>(args)...);
			return ToHandle(key);
		}

		Handle Allocate()
		{
			const auto key = m_storage.Emplace();
			return ToHandle(key);
		}

		Handle Append(DataType&& data)
		{
			const auto key = m_storage.Insert(std::move(data));
			return ToHandle(key);
		}

		[[nodiscard]] bool Has(const Handle handle) const
		{
			if (!IsMatchingHandle(handle))
				return false;
			return m_storage.Contains(ToSlotKey(handle));
		}

		[[nodiscard]] DataType* Get(const Handle handle)
		{
			if (!IsMatchingHandle(handle))
				throw std::runtime_error("Invalid handle!");

			DataType* ptr = m_storage.Get(ToSlotKey(handle));
			if (!ptr)
				throw std::runtime_error("Invalid handle!");
			return ptr;
		}

		[[nodiscard]] const DataType* Get(const Handle handle) const
		{
			if (!IsMatchingHandle(handle))
				throw std::runtime_error("Invalid handle!");

			const DataType* ptr = m_storage.Get(ToSlotKey(handle));
			if (!ptr)
				throw std::runtime_error("Invalid handle!");
			return ptr;
		}

		void Destroy(const std::uint32_t index)
		{
			m_storage.EraseBySlot(index);
		}

		void Destroy(const Handle handle)
		{
			if (!IsMatchingHandle(handle) || !m_storage.Erase(ToSlotKey(handle)))
				throw std::runtime_error("Invalid handle!");
		}

		void Clear() noexcept
		{
			m_storage.Clear();
		}

		[[nodiscard]] std::uint32_t Size() const noexcept
		{
			return static_cast<std::uint32_t>(m_storage.Size());
		}

		[[nodiscard]] bool IsActive(const std::uint32_t index) const noexcept
		{
			return m_storage.IsSlotOccupied(index);
		}

		[[nodiscard]] DataType* Get(const std::uint32_t index)
		{
			DataType* ptr = m_storage.GetBySlot(index);
			if (!ptr)
				throw std::out_of_range("HandleRegistry: inactive or out-of-range index");
			return ptr;
		}

		template <typename Fn>
		void ForEachActive(Fn&& fn)
		{
			m_storage.ForEach([&](const typename Storage::key_type key, DataType& value) {
				fn(key.index, value);
			});
		}

	private:
		static constexpr uint32_t MaxGeneration = 0x00FFFFFFu;
		using Storage = SlotMap<DataType, void, MaxGeneration>;

		[[nodiscard]] static bool IsMatchingHandle(const Handle handle) noexcept
		{
			return handle.IsValid() && handle.GetType() == static_cast<uint8_t>(Type);
		}

		[[nodiscard]] static typename Storage::key_type ToSlotKey(const Handle handle) noexcept
		{
			return {handle.GetId(), handle.GetGeneration()};
		}

		[[nodiscard]] static Handle ToHandle(const typename Storage::key_type key) noexcept
		{
			return Handle(key.index, key.generation, static_cast<uint8_t>(Type));
		}

		Storage m_storage;
	};
}
