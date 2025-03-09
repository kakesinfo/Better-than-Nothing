//
// Created by acoezard on 11/25/24.
//

#pragma once

namespace BetterThanNothing
{
	class CApplicationContext;

	enum class EVulkanBufferType;
	class CVulkanBuffer;

	class CVulkanBufferFactory
	{
	private:
		CApplicationContext* m_context;

	public:
		explicit CVulkanBufferFactory(CApplicationContext* context): m_context(context) {}
		~CVulkanBufferFactory() = default;

		CVulkanBuffer CreateVertexBuffer(const void* data, uint32 dataSize) const;
		CVulkanBuffer CreateIndexBuffer(const void* data, uint32 dataSize) const;
	};
}
