//
// Created by acoezard on 11/25/24.
//

#pragma once

namespace BetterThanNothing
{
	struct CVulkanContext;

	enum class EVulkanBufferType;
	class CVulkanBuffer;

	class CVulkanBufferFactory
	{
	private:
		static CVulkanContext* m_context;

	public:
		static void Initialize(CVulkanContext* context);
		static void Shutdown();

		static CVulkanBuffer CreateVertexBuffer(const void* data, uint32 dataSize);
		static CVulkanBuffer CreateIndexBuffer(const void* data, uint32 dataSize);
	};
}
