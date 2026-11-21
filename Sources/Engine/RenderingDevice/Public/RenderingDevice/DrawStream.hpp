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
#include "Types.hpp"

#include <Common/EShaderType.hpp>
#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace BetterThanNothing
{
	class DrawStreamView;
	class DrawStreamViewIterator;

	struct DrawPacket
	{
		static constexpr uint32_t MaxDescriptorSets = 4;
		static constexpr uint32_t MaxPushConstantSize = 128;

		Handle									   pipeline{};
		std::string								   pipelineLayoutName;
		Handle									   vertexBuffer{};
		Handle									   indexBuffer{};
		uint32_t								   indexCount = 0;
		uint32_t								   instanceCount = 1;
		std::array<Handle, MaxDescriptorSets>	   descriptorSets{};
		uint32_t								   descriptorSetCount = 0;
		std::array<std::byte, MaxPushConstantSize> pushConstantData{};
		uint32_t								   pushConstantSize = 0;
		ShaderStageFlags						   pushConstantStages = ShaderStageFlags::Vertex;
		float									   distanceToCamera = 0.0f;
		bool									   sortBackToFront = false;

		template <typename T>
		void SetPushConstant(const T& value, const ShaderStageFlags stages = ShaderStageFlags::Vertex)
		{
			static_assert(std::is_trivially_copyable_v<T>, "Push constant data must be trivially copyable");
			static_assert(sizeof(T) <= MaxPushConstantSize, "Push constant exceeds 128-byte minimum Vulkan guarantee");
			std::memcpy(pushConstantData.data(), &value, sizeof(T));
			pushConstantSize = static_cast<uint32_t>(sizeof(T));
			pushConstantStages = stages;
		}
	};

	struct DrawPacketSortKey
	{
		uint32_t packetId = 0;
		uint64_t key = 0;

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
		[[nodiscard]] bool			 Empty() const { return m_packets.empty(); }
		[[nodiscard]] size_t		 Size() const { return m_packets.size(); }

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
