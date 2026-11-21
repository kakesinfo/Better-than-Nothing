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

#include "RenderingDevice/DrawStream.hpp"
#include <algorithm>
#include <bit>

namespace BetterThanNothing
{
	bool DrawPacketSortKey::operator<(const DrawPacketSortKey& other) const
	{
		return key < other.key;
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
		const auto packetIndex = static_cast<uint32_t>(m_packets.size());
		m_packets.push_back(drawPacket);

		const float	   clampedDist = drawPacket.distanceToCamera > 0.0f ? drawPacket.distanceToCamera : 0.0f;
		const uint64_t quantizedDist = static_cast<uint64_t>(std::bit_cast<uint32_t>(clampedDist) >> 16) & 0xFFFFu;
		const uint64_t meshId = static_cast<uint64_t>(drawPacket.vertexBuffer.GetId()) & 0xFFFFu;
		const uint64_t textureId = static_cast<uint64_t>(drawPacket.samplerDescriptorSet.GetId()) & 0xFFFFu;
		const uint64_t pipelineId = static_cast<uint64_t>(drawPacket.pipelineHandle.GetId()) & 0xFFFFu;

		DrawPacketSortKey sortKey{};
		sortKey.packetId = packetIndex;
		sortKey.key = (pipelineId << 48) | (textureId << 32) | (meshId << 16) | quantizedDist;

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
		return m_packets[m_currentKey->packetId];
	}

	DrawPacket* DrawStreamViewIterator::operator->() const
	{
		return &m_packets[m_currentKey->packetId];
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
