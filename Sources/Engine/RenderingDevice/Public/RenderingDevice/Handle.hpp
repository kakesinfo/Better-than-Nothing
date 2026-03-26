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

#include <memory>

namespace BetterThanNothing
{
	enum class HandleType: uint8_t
	{
		INVALID = 0,
		DescriptorSet,
		Buffer,
		Texture,
		Sampler,
		Shader
	};

	class Handle
	{
	public:
		Handle() = default;
		Handle(const uint32_t index, const uint32_t generation, const uint8_t type) : m_id(index), m_generation(generation), m_type(type)
		{}

		[[nodiscard]] uint32_t GetId() const { return m_id; }
		[[nodiscard]] uint32_t GetGeneration() const { return m_generation; }
		[[nodiscard]] uint8_t GetType() const { return m_type; }
		[[nodiscard]] bool	   IsValid() const { return m_id != 0 && m_generation != 0 && m_type != 0; }

		bool operator==(const Handle& other) const
		{
			return m_id == other.m_id && m_generation == other.m_generation && m_type == other.m_type;
		}

		bool operator!=(const Handle& other) const
		{
			return m_id != other.GetId() || m_generation != other.GetGeneration() || m_type != other.GetType();
		}

		bool operator<(const Handle& other) const
		{
			return m_id < other.GetId() || (m_id == other.GetId() && m_generation < other.GetGeneration());
		}

		bool operator>(const Handle& other) const
		{
			return m_id > other.GetId() || (m_id == other.GetId() && m_generation > other.GetGeneration());
		}

	private:
		uint64_t m_id : 32 = 0;
		uint64_t m_generation : 24 = 0;
		uint64_t m_type : 8 = 0;
	};
}

namespace std
{
	template<>
	struct hash<BetterThanNothing::Handle>
	{
		size_t operator()(const BetterThanNothing::Handle& h) const noexcept
		{
			return hash<uint32_t>()(h.GetId()) ^ (hash<uint32_t>()(h.GetGeneration()) << 1) ^ (hash<uint8_t>()(h.GetType()) << 2);
		}
	};
}
