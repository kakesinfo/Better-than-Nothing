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
	namespace RenderingApi
	{
		class Allocator;
	}

	class StagingBuffer
	{
	public:
		/// @param allocator The allocator to use to create/destroy the staging buffer.
		/// @param size The staging buffer size in megabytes.
		explicit StagingBuffer(RenderingApi::Allocator* allocator, uint64_t size);
		~StagingBuffer();

		StagingBuffer(const StagingBuffer&) = delete;
		StagingBuffer& operator=(const StagingBuffer&) = delete;
		StagingBuffer(StagingBuffer&& other) = delete;
		StagingBuffer& operator=(StagingBuffer&& other) = delete;

		[[nodiscard]] static std::unique_ptr<StagingBuffer> Create(RenderingApi::Allocator* allocator, uint64_t size);

		uint64_t			   Stage(const void* data, uint64_t dataSize, uint32_t alignment = 4);
		[[nodiscard]] VkBuffer GetBuffer() const;
		void				   Reset();


	private:
		RenderingApi::Allocator* m_allocator;
		VkBuffer				 m_buffer = VK_NULL_HANDLE;
		VmaAllocation			 m_bufferAlloc = VK_NULL_HANDLE;
		VmaAllocationInfo		 m_bufferAllocInfo = {};
		uint64_t				 m_size = 0;
		uint64_t				 m_offset = 0;
	};
}
