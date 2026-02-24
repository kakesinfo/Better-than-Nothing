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

#include <Module_Math/Matrix4.hpp>
#include <vector>

namespace BetterThanNothing
{
	class DrawStreamView;
	class DrawStreamViewIterator;

	struct DrawPacket
	{
		VkPipeline		 pipeline{};
		VkPipelineLayout pipelineLayout{};
		BufferHandle	 vertexBuffer{};
		BufferHandle	 indexBuffer{};
		uint32_t		 indexCount{};
		VkDescriptorSet	 globalDescriptorSet{};
		VkDescriptorSet	 samplerDescriptorSet{};
		VkDescriptorSet	 pointLightDescriptorSet{};
		Matrix4			 transform;
		float			 distanceToCamera{};
	};

	struct DrawPacketSortKeyData
	{
		uint64_t packetId : 32 = 0;
		uint64_t distance : 24 = 0;
		uint64_t _padding : 8 = 0;
	};

	struct DrawPacketSortKey
	{
		DrawPacketSortKeyData data;

		bool operator<(const DrawPacketSortKey& other) const;
	};

	class DrawStream
	{
	public:
		DrawStream() = default;
		~DrawStream() = default;

		void						 Reset(std::uint32_t newSize);
		void						 Draw(const DrawPacket& drawPacket);
		[[nodiscard]] DrawStreamView Build();

	private:
		std::vector<DrawPacket>		   m_packets;
		std::vector<DrawPacketSortKey> m_sortKeys;
	};

	class DrawStreamView
	{
	public:
		using Iterator = DrawStreamViewIterator;

		DrawStreamView(DrawPacket* packets, DrawPacketSortKey* keys, size_t count) : m_packets(packets), m_keys(keys), m_count(count) {}
		~DrawStreamView() = default;

		[[nodiscard]] Iterator begin() const;
		[[nodiscard]] Iterator end() const;

	private:
		DrawPacket*		   m_packets;
		DrawPacketSortKey* m_keys;
		size_t			   m_count;
	};

	class DrawStreamViewIterator
	{
	public:
		DrawStreamViewIterator(DrawPacketSortKey* currentKey, DrawPacket* packets) : m_currentKey(currentKey), m_packets(packets) {}
		~DrawStreamViewIterator() = default;

		DrawPacket&				operator*() const;
		DrawPacket*				operator->() const;
		DrawStreamViewIterator& operator++();
		DrawStreamViewIterator	operator++(int);

		bool operator==(const DrawStreamViewIterator& other) const;
		bool operator!=(const DrawStreamViewIterator& other) const;

	private:
		DrawPacketSortKey* m_currentKey;
		DrawPacket*		   m_packets;
	};
}
