#pragma once


namespace BetterThanNothing
{
	struct VulkanContext;
	class CVulkanVertexBuffer;
	class CVulkanIndexBuffer;
	class CVulkanUniformBuffer;

	class CBufferManager
	{
	public:
		static void Initialize(VulkanContext* context);
		static void Shutdown();

		static std::unique_ptr<CVulkanVertexBuffer> CreateVertexBuffer(const void* data, std::uint32_t dataSize);
		static std::unique_ptr<CVulkanIndexBuffer> CreateIndexBuffer(const void* data, std::uint32_t dataSize);

		template <typename T>
		static std::unique_ptr<CVulkanUniformBuffer> CreateUniformBuffer()
		{
			return CreateUniformBuffer(sizeof(T));
		}

		static std::unique_ptr<CVulkanUniformBuffer> CreateUniformBuffer(std::uint32_t dataSize);

	private:
		static VulkanContext* m_context;
	};
}
