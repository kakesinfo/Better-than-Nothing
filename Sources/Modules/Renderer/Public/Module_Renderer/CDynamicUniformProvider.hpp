#pragma once

namespace BetterThanNothing
{
	class CDynamicUniformProvider
	{
	public:
		explicit CDynamicUniformProvider(VulkanContext* pContext, std::uint32_t maxObjects);
		~CDynamicUniformProvider() = default;

		template <typename T>
		T* Get(const std::uint32_t frameIndex, const std::uint32_t objectIndex) const
		{
			char* base = static_cast<char*>(m_pContext->vkFrames[frameIndex].dynamicUniformBuffer->GetData());
			return reinterpret_cast<T*>(base + objectIndex * m_uniformAlignment);
		}

		std::uint32_t GetOffset(const std::uint32_t objectIndex) const
		{
			return objectIndex * m_uniformAlignment;
		}

	private:
		std::uint32_t _calculateUniformAlignment() const;

		VulkanContext* m_pContext = nullptr;
		std::uint32_t m_maxObjects;
		std::uint32_t m_uniformAlignment;
	};
};