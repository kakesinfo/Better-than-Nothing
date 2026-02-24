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

		Handle<RenderingApi::Buffer> CreateBuffer(const BufferDescription& desc);
		Handle<RenderingApi::Buffer> CreateVertexBuffer(const void* data, size_t size);
		Handle<RenderingApi::Buffer> CreateIndexBuffer(const void* data, size_t size);
		bool						 CopyDataToBuffer(Handle<RenderingApi::Buffer> dstBufferHandle, const void* data, size_t size);
		void						 DestroyBuffer(Handle<RenderingApi::Buffer> handle);
		RenderingApi::Buffer*		 GetBuffer(Handle<RenderingApi::Buffer> handle);

	private:
		BufferRegistryCreateInfo			 m_createInfo;
		HandleRegistry<RenderingApi::Buffer> m_buffers;
	};
}
