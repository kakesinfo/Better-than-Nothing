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

#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace BetterThanNothing
{
	/**
	 * Generational key identifying an element inside a SlotMap<T, Tag>.
	 * generation == 0 denotes an invalid/null key.
	 */
	template <typename Tag = void>
	struct SlotKey
	{
		uint32_t index = 0;
		uint32_t generation = 0;

		[[nodiscard]] constexpr bool IsValid() const noexcept { return generation != 0; }
		constexpr explicit			 operator bool() const noexcept { return IsValid(); }

		constexpr bool				   operator==(const SlotKey& other) const noexcept = default;
		constexpr std::strong_ordering operator<=>(const SlotKey& other) const noexcept = default;
	};

	/**
	 * Dense generational SlotMap:
	 * - O(1) insertion, lookup, and swap-and-pop erasure
	 * - ABA-safe keys via generation counters
	 * - Contiguous dense storage for cache-friendly iteration
	 */
	template <typename T, typename Tag = void, uint32_t MaxGeneration = 0xFFFFFFFFu>
	class SlotMap
	{
		static_assert(MaxGeneration >= 1u, "MaxGeneration must be at least 1");

	public:
		using value_type = T;
		using key_type = SlotKey<Tag>;
		using size_type = std::size_t;
		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;

		SlotMap() = default;
		~SlotMap() = default;

		SlotMap(const SlotMap&) = default;
		SlotMap& operator=(const SlotMap&) = default;
		SlotMap(SlotMap&&) noexcept = default;
		SlotMap& operator=(SlotMap&&) noexcept = default;

		template <typename... Args>
		[[nodiscard]] key_type Emplace(Args&&... args)
		{
			uint32_t sparseIdx = 0;
			if (!m_freeList.empty())
			{
				sparseIdx = m_freeList.back();
				m_freeList.pop_back();
			}
			else
			{
				sparseIdx = static_cast<uint32_t>(m_sparse.size());
				m_sparse.emplace_back();
			}

			const auto denseIdx = static_cast<uint32_t>(m_dense.size());
			m_dense.emplace_back(std::forward<Args>(args)...);
			m_denseToSparse.push_back(sparseIdx);

			SparseSlot& slot = m_sparse[sparseIdx];
			slot.denseIndex = denseIdx;
			slot.occupied = true;

			return key_type{sparseIdx, slot.generation};
		}

		[[nodiscard]] key_type Insert(const T& value)
		{
			return Emplace(value);
		}

		[[nodiscard]] key_type Insert(T&& value)
		{
			return Emplace(std::move(value));
		}

		bool Erase(const key_type key) noexcept
		{
			if (!Contains(key))
				return false;

			return EraseBySlot(key.index);
		}

		bool EraseBySlot(const uint32_t sparseIndex) noexcept
		{
			if (sparseIndex >= m_sparse.size())
				return false;

			SparseSlot& slot = m_sparse[sparseIndex];
			if (!slot.occupied)
				return false;

			const uint32_t denseIdx = slot.denseIndex;
			const uint32_t lastDenseIdx = static_cast<uint32_t>(m_dense.size() - 1);

			if (denseIdx != lastDenseIdx)
			{
				m_dense[denseIdx] = std::move(m_dense[lastDenseIdx]);
				const uint32_t movedSparseIdx = m_denseToSparse[lastDenseIdx];
				m_denseToSparse[denseIdx] = movedSparseIdx;
				m_sparse[movedSparseIdx].denseIndex = denseIdx;
			}

			m_dense.pop_back();
			m_denseToSparse.pop_back();

			slot.occupied = false;
			slot.generation = NextGeneration(slot.generation);
			m_freeList.push_back(sparseIndex);
			return true;
		}

		[[nodiscard]] bool Contains(const key_type key) const noexcept
		{
			if (!key.IsValid() || key.index >= m_sparse.size())
				return false;

			const SparseSlot& slot = m_sparse[key.index];
			return slot.occupied && slot.generation == key.generation;
		}

		[[nodiscard]] bool IsSlotOccupied(const uint32_t sparseIndex) const noexcept
		{
			return sparseIndex < m_sparse.size() && m_sparse[sparseIndex].occupied;
		}

		[[nodiscard]] T* Get(const key_type key) noexcept
		{
			if (!Contains(key))
				return nullptr;
			return &m_dense[m_sparse[key.index].denseIndex];
		}

		[[nodiscard]] const T* Get(const key_type key) const noexcept
		{
			if (!Contains(key))
				return nullptr;
			return &m_dense[m_sparse[key.index].denseIndex];
		}

		[[nodiscard]] T* GetBySlot(const uint32_t sparseIndex) noexcept
		{
			if (!IsSlotOccupied(sparseIndex))
				return nullptr;
			return &m_dense[m_sparse[sparseIndex].denseIndex];
		}

		[[nodiscard]] const T* GetBySlot(const uint32_t sparseIndex) const noexcept
		{
			if (!IsSlotOccupied(sparseIndex))
				return nullptr;
			return &m_dense[m_sparse[sparseIndex].denseIndex];
		}

		[[nodiscard]] T& At(const key_type key)
		{
			T* ptr = Get(key);
			if (!ptr)
				throw std::out_of_range("SlotMap: invalid or stale key");
			return *ptr;
		}

		[[nodiscard]] const T& At(const key_type key) const
		{
			const T* ptr = Get(key);
			if (!ptr)
				throw std::out_of_range("SlotMap: invalid or stale key");
			return *ptr;
		}

		[[nodiscard]] key_type KeyAtDenseIndex(const size_type denseIndex) const noexcept
		{
			assert(denseIndex < m_denseToSparse.size() && "SlotMap dense index out of range");
			const uint32_t sparseIdx = m_denseToSparse[denseIndex];
			return key_type{sparseIdx, m_sparse[sparseIdx].generation};
		}

		void Reserve(const size_type capacity)
		{
			m_sparse.reserve(capacity);
			m_dense.reserve(capacity);
			m_denseToSparse.reserve(capacity);
		}

		void Clear() noexcept
		{
			for (const uint32_t sparseIdx : m_denseToSparse)
			{
				SparseSlot& slot = m_sparse[sparseIdx];
				slot.occupied = false;
				slot.generation = NextGeneration(slot.generation);
				m_freeList.push_back(sparseIdx);
			}
			m_dense.clear();
			m_denseToSparse.clear();
		}

		template <typename Fn>
		void ForEach(Fn&& fn)
		{
			// Iterate backwards so erasing the current element inside the callback is safe
			for (size_type i = m_dense.size(); i > 0; --i)
			{
				const size_type denseIdx = i - 1;
				if constexpr (std::invocable<Fn, key_type, T&>)
					fn(KeyAtDenseIndex(denseIdx), m_dense[denseIdx]);
				else
					fn(m_dense[denseIdx]);
			}
		}

		template <typename Fn>
		void ForEach(Fn&& fn) const
		{
			for (size_type i = 0; i < m_dense.size(); ++i)
			{
				if constexpr (std::invocable<Fn, key_type, const T&>)
					fn(KeyAtDenseIndex(i), m_dense[i]);
				else
					fn(m_dense[i]);
			}
		}

		[[nodiscard]] size_type Size() const noexcept { return m_dense.size(); }
		[[nodiscard]] size_type Capacity() const noexcept { return m_dense.capacity(); }
		[[nodiscard]] bool		Empty() const noexcept { return m_dense.empty(); }

		[[nodiscard]] T*			 Data() noexcept { return m_dense.data(); }
		[[nodiscard]] const T*		 Data() const noexcept { return m_dense.data(); }
		[[nodiscard]] T*			 data() noexcept { return m_dense.data(); }
		[[nodiscard]] const T*		 data() const noexcept { return m_dense.data(); }
		[[nodiscard]] size_type		 size() const noexcept { return m_dense.size(); }
		[[nodiscard]] bool			 empty() const noexcept { return m_dense.empty(); }
		[[nodiscard]] iterator		 begin() noexcept { return m_dense.begin(); }
		[[nodiscard]] iterator		 end() noexcept { return m_dense.end(); }
		[[nodiscard]] const_iterator begin() const noexcept { return m_dense.begin(); }
		[[nodiscard]] const_iterator end() const noexcept { return m_dense.end(); }
		[[nodiscard]] const_iterator cbegin() const noexcept { return m_dense.cbegin(); }
		[[nodiscard]] const_iterator cend() const noexcept { return m_dense.cend(); }

	private:
		struct SparseSlot
		{
			uint32_t denseIndex = 0;
			uint32_t generation = 1;
			bool	 occupied = false;
		};

		[[nodiscard]] static constexpr uint32_t NextGeneration(const uint32_t current) noexcept
		{
			return current >= MaxGeneration ? 1u : current + 1u;
		}

		std::vector<SparseSlot> m_sparse;
		std::vector<T>			m_dense;
		std::vector<uint32_t>	m_denseToSparse;
		std::vector<uint32_t>	m_freeList;
	};
}

namespace std
{
	template <typename Tag>
	struct hash<BetterThanNothing::SlotKey<Tag>>
	{
		size_t operator()(const BetterThanNothing::SlotKey<Tag>& key) const noexcept
		{
			const uint64_t packed = (static_cast<uint64_t>(key.generation) << 32) | static_cast<uint64_t>(key.index);
			return hash<uint64_t>{}(packed);
		}
	};
}
