#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CDynamicUniformProvider
	{
	public:
		explicit CDynamicUniformProvider(VulkanContext* pContext, uint32 maxObjects);
		~CDynamicUniformProvider() = default;

		template <typename T>
		T* Get(const uint32 frameIndex, const uint32 objectIndex) const
		{
			char* base = static_cast<char*>(m_pContext->vkFrames[frameIndex].dynamicUniformBuffer->GetData());
			return reinterpret_cast<T*>(base + objectIndex * m_uniformAlignment);
		}

		uint32 GetOffset(const uint32 objectIndex) const
		{
			return objectIndex * m_uniformAlignment;
		}

	private:
		uint32 _calculateUniformAlignment() const;

		VulkanContext* m_pContext = nullptr;
		uint32 m_maxObjects;
		uint32 m_uniformAlignment;
	};
};