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

#include "RenderingDevice/DrawStream.hpp"
#include <algorithm>
#include <bit>

namespace BetterThanNothing
{
	bool DrawPacketSortKey::operator<(const DrawPacketSortKey& other) const
	{
		return std::bit_cast<uint64_t>(data) < std::bit_cast<uint64_t>(other.data);
	}

	void DrawStream::Reset(const std::uint32_t newSize)
	{
		m_packets.clear();
		if (newSize > m_packets.capacity())
			m_packets.reserve(newSize);

		m_sortKeys.clear();
		if (newSize > m_sortKeys.capacity())
			m_sortKeys.reserve(newSize);
	}

	void DrawStream::Draw(const DrawPacket& drawPacket)
	{
		m_packets.push_back(drawPacket);

		DrawPacketSortKey sortKey{};
		sortKey.data.packetId = m_packets.size() - 1;
		sortKey.data.distance = std::bit_cast<uint32_t>(std::max(0.0f, drawPacket.distanceToCamera)) >> 8;

		m_sortKeys.push_back(sortKey);
	}

	DrawStreamView DrawStream::Build()
	{
		std::sort(m_sortKeys.begin(), m_sortKeys.end());

		return DrawStreamView{m_packets.data(), m_sortKeys.data(), m_packets.size()};
	}

	DrawStreamView::Iterator DrawStreamView::begin() const
	{
		return {m_keys, m_packets};
	}

	DrawStreamView::Iterator DrawStreamView::end() const
	{
		return {m_keys + m_count, m_packets};
	}

	DrawPacket& DrawStreamViewIterator::operator*() const
	{
		return m_packets[m_currentKey->data.packetId];
	}

	DrawPacket* DrawStreamViewIterator::operator->() const
	{
		return &m_packets[m_currentKey->data.packetId];
	}

	DrawStreamViewIterator& DrawStreamViewIterator::operator++()
	{
		++m_currentKey;
		return *this;
	}

	DrawStreamViewIterator DrawStreamViewIterator::operator++(int)
	{
		const DrawStreamViewIterator tmp = *this;
		++(*this);
		return tmp;
	}

	bool DrawStreamViewIterator::operator==(const DrawStreamViewIterator& other) const
	{
		return m_currentKey == other.m_currentKey;
	}

	bool DrawStreamViewIterator::operator!=(const DrawStreamViewIterator& other) const
	{
		return m_currentKey != other.m_currentKey;
	}
}
