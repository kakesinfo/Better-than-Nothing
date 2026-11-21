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
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace BetterThanNothing
{
	/**
	 * Stack-allocated dynamic-length array with a fixed compile-time capacity
	 * (similar to C++26 std::inplace_vector). Performs zero heap allocations.
	 */
	template <typename T, std::size_t MaxCapacity>
	class FixedArray
	{
		static_assert(MaxCapacity > 0, "FixedArray capacity must be greater than 0");

	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using iterator = T*;
		using const_iterator = const T*;

		FixedArray() noexcept = default;

		FixedArray(std::initializer_list<T> init)
			requires std::copy_constructible<T>
		{
			assert(init.size() <= MaxCapacity && "FixedArray initializer_list exceeds capacity");
			for (const T& item : init)
				Emplace(item);
		}

		FixedArray(const FixedArray& other)
			requires std::copy_constructible<T>
		{
			for (size_type i = 0; i < other.m_size; ++i)
				Emplace(other[i]);
		}

		FixedArray(FixedArray&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
			requires std::move_constructible<T>
		{
			for (size_type i = 0; i < other.m_size; ++i)
				Emplace(std::move(other[i]));
			other.Clear();
		}

		FixedArray& operator=(const FixedArray& other)
			requires std::copy_constructible<T>
		{
			if (this != &other)
			{
				Clear();
				for (size_type i = 0; i < other.m_size; ++i)
					Emplace(other[i]);
			}
			return *this;
		}

		FixedArray& operator=(FixedArray&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
			requires std::move_constructible<T>
		{
			if (this != &other)
			{
				Clear();
				for (size_type i = 0; i < other.m_size; ++i)
					Emplace(std::move(other[i]));
				other.Clear();
			}
			return *this;
		}

		FixedArray& operator=(std::initializer_list<T> init)
			requires std::copy_constructible<T>
		{
			assert(init.size() <= MaxCapacity && "FixedArray initializer_list exceeds capacity");
			Clear();
			for (const T& item : init)
				Emplace(item);
			return *this;
		}

		~FixedArray() noexcept
		{
			Clear();
		}

		template <typename... Args>
		reference Emplace(Args&&... args)
		{
			assert(m_size < MaxCapacity && "FixedArray is full");
			T* ptr = std::construct_at(SlotPtr(m_size), std::forward<Args>(args)...);
			++m_size;
			return *ptr;
		}

		void Push(const value_type& value)
		{
			Emplace(value);
		}

		void Push(value_type&& value)
		{
			Emplace(std::move(value));
		}

		void Pop() noexcept
		{
			assert(m_size > 0 && "FixedArray is empty");
			--m_size;
			std::destroy_at(Data() + m_size);
		}

		void Clear() noexcept
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				for (size_type i = m_size; i > 0; --i)
					std::destroy_at(Data() + (i - 1));
			}
			m_size = 0;
		}

		void Resize(const size_type newSize)
			requires std::default_initializable<T>
		{
			assert(newSize <= MaxCapacity && "FixedArray resize exceeds capacity");
			while (m_size < newSize)
				Emplace();
			while (m_size > newSize)
				Pop();
		}

		[[nodiscard]] reference At(const size_type index)
		{
			assert(index < m_size && "FixedArray index out of range");
			return Data()[index];
		}

		[[nodiscard]] const_reference At(const size_type index) const
		{
			assert(index < m_size && "FixedArray index out of range");
			return Data()[index];
		}

		[[nodiscard]] reference operator[](const size_type index) noexcept
		{
			assert(index < m_size && "FixedArray index out of range");
			return Data()[index];
		}

		[[nodiscard]] const_reference operator[](const size_type index) const noexcept
		{
			assert(index < m_size && "FixedArray index out of range");
			return Data()[index];
		}

		[[nodiscard]] reference Front() noexcept
		{
			assert(m_size > 0 && "FixedArray is empty");
			return Data()[0];
		}

		[[nodiscard]] const_reference Front() const noexcept
		{
			assert(m_size > 0 && "FixedArray is empty");
			return Data()[0];
		}

		[[nodiscard]] reference Back() noexcept
		{
			assert(m_size > 0 && "FixedArray is empty");
			return Data()[m_size - 1];
		}

		[[nodiscard]] const_reference Back() const noexcept
		{
			assert(m_size > 0 && "FixedArray is empty");
			return Data()[m_size - 1];
		}

		[[nodiscard]] pointer Data() noexcept
		{
			return std::launder(reinterpret_cast<pointer>(m_data));
		}

		[[nodiscard]] const_pointer Data() const noexcept
		{
			return std::launder(reinterpret_cast<const_pointer>(m_data));
		}

		[[nodiscard]] static constexpr size_type Capacity() noexcept { return MaxCapacity; }
		[[nodiscard]] size_type					 Size() const noexcept { return m_size; }
		[[nodiscard]] bool						 Empty() const noexcept { return m_size == 0; }
		[[nodiscard]] bool						 Full() const noexcept { return m_size == MaxCapacity; }

		// STL / ranges / std::span compatibility
		[[nodiscard]] pointer		 data() noexcept { return Data(); }
		[[nodiscard]] const_pointer	 data() const noexcept { return Data(); }
		[[nodiscard]] size_type		 size() const noexcept { return m_size; }
		[[nodiscard]] bool			 empty() const noexcept { return m_size == 0; }
		[[nodiscard]] iterator		 begin() noexcept { return Data(); }
		[[nodiscard]] iterator		 end() noexcept { return Data() + m_size; }
		[[nodiscard]] const_iterator begin() const noexcept { return Data(); }
		[[nodiscard]] const_iterator end() const noexcept { return Data() + m_size; }
		[[nodiscard]] const_iterator cbegin() const noexcept { return Data(); }
		[[nodiscard]] const_iterator cend() const noexcept { return Data() + m_size; }

	private:
		[[nodiscard]] pointer SlotPtr(const size_type index) noexcept
		{
			return reinterpret_cast<pointer>(m_data) + index;
		}

		alignas(T) std::byte m_data[MaxCapacity * sizeof(T)];
		size_type			 m_size = 0;
	};
}
