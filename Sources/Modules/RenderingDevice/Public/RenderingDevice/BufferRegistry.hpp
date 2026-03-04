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

#include "BufferDescription.hpp"
#include "Handle.hpp"
#include "HandleRegistry.hpp"

namespace BetterThanNothing
{
	namespace RenderingApi
	{
		class Device;
		class CommandPool;
		class Queue;
		class Allocator;
		struct Buffer;
	}

	struct BufferRegistryCreateInfo
	{
		RenderingApi::Device*	   device;
		RenderingApi::CommandPool* commandPool;
		RenderingApi::Queue*	   graphicsQueue;
		RenderingApi::Allocator*   allocator;
	};

	class BufferRegistry
	{
	public:
		explicit BufferRegistry(const BufferRegistryCreateInfo& createInfo);
		~BufferRegistry();

		static std::unique_ptr<BufferRegistry> Create(const BufferRegistryCreateInfo& createInfo);

		Handle CreateBuffer(const BufferDescription& desc);
		Handle CreateVertexBuffer(const void* data, size_t size);
		Handle CreateIndexBuffer(const void* data, size_t size);
		bool						 CopyDataToBuffer(Handle dstBufferHandle, const void* data, size_t size);
		void						 DestroyBuffer(Handle handle);
		RenderingApi::Buffer*		 GetBuffer(Handle handle);

	private:
		BufferRegistryCreateInfo			 m_createInfo;
		HandleRegistry<RenderingApi::Buffer, HandleType::Buffer> m_buffers;
	};
}
