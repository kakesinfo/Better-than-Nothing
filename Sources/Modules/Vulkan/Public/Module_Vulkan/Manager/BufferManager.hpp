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

namespace BetterThanNothing
{
	struct VulkanContext;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanUniformBuffer;

	class BufferManager
	{
	public:
		static void Initialize(VulkanContext* context);
		static void Shutdown();

		static std::unique_ptr<VulkanVertexBuffer> CreateVertexBuffer(const void* data, std::uint32_t dataSize);
		static std::unique_ptr<VulkanIndexBuffer>  CreateIndexBuffer(const void* data, std::uint32_t dataSize);
		static std::unique_ptr<VulkanStorageBuffer> CreateStorageBuffer(std::uint32_t dataSize);

		template <typename T>
		static std::unique_ptr<VulkanUniformBuffer> CreateUniformBuffer()
		{
			return CreateUniformBuffer(sizeof(T));
		}

		static std::unique_ptr<VulkanUniformBuffer> CreateUniformBuffer(std::uint32_t dataSize);

	private:
		static VulkanContext* m_context;
	};
}
