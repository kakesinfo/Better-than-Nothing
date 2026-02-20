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

#pragma once

#include <memory>

namespace BetterThanNothing
{
	template <typename T>
	class Handle
	{
	public:
		Handle() = default;
		Handle(const uint32_t index, const uint32_t generation) : m_id(index), m_generation(generation) {}

		[[nodiscard]] std::uint32_t GetId() const { return m_id; }
		[[nodiscard]] std::uint32_t GetGeneration() const { return m_generation; }
		[[nodiscard]] bool			IsValid() const { return m_id != 0 && m_generation != 0; }

		bool operator==(const Handle& other) const { return m_id == other.m_id && m_generation == other.m_generation; }
		bool operator!=(const Handle& other) const { return m_id != other.GetId() || m_generation != other.GetGeneration(); }
		bool operator<(const Handle& other) const
		{
			return m_id < other.GetId() || (m_id == other.GetId() && m_generation < other.GetGeneration());
		}
		bool operator>(const Handle& other) const
		{
			return m_id > other.GetId() || (m_id == other.GetId() && m_generation > other.GetGeneration());
		}

	private:
		std::uint32_t m_id : 20 = 0;
		std::uint32_t m_generation : 12 = 0;
	};

	namespace RenderingApi
	{
		struct Sampler;
		struct Buffer;
	}

	using SamplerHandle = Handle<RenderingApi::Sampler>;
	using BufferHandle = Handle<RenderingApi::Buffer>;
}

namespace std
{
	template <typename T>
	struct hash<BetterThanNothing::Handle<T>>
	{
		size_t operator()(const BetterThanNothing::Handle<T>& h) const
		{
			return hash<uint32_t>()(h.GetId()) ^ (hash<uint32_t>()(h.GetGeneration()) << 1);
		}
	};
}
