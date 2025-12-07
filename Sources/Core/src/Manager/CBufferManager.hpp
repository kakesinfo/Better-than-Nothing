//
// Created by acoezard on 11/25/24.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct VulkanContext;

	class CBufferManager
	{
	public:
		static void Initialize(VulkanContext* context);
		static void Shutdown();

		static std::unique_ptr<CVulkanVertexBuffer> CreateVertexBuffer(const void* data, uint32 dataSize);
		static std::unique_ptr<CVulkanIndexBuffer> CreateIndexBuffer(const void* data, uint32 dataSize);

		template <typename T>
		static std::unique_ptr<CVulkanUniformBuffer> CreateUniformBuffer()
		{
			return CreateUniformBuffer(sizeof(T));
		}

		static std::unique_ptr<CVulkanUniformBuffer> CreateUniformBuffer(uint32 dataSize);


	private:
		static VulkanContext* m_context;
	};
}
