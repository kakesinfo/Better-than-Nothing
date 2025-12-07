#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CGlobalUniformProvider
	{
	public:
		explicit CGlobalUniformProvider(VulkanContext* pContext);
		~CGlobalUniformProvider() = default;

		template <typename T>
		T* Get(const uint32 frameIndex) const
		{
			return static_cast<T*>(m_pContext->vkFrames[frameIndex].globalUniformBuffer->GetData());
		}

	private:
		VulkanContext* m_pContext = nullptr;
	};
};