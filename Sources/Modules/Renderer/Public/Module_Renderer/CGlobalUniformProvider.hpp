#pragma once

#include <module_vulkan.hpp>

namespace BetterThanNothing
{


	class CGlobalUniformProvider
	{
	public:
		explicit CGlobalUniformProvider(VulkanContext* pContext);
		~CGlobalUniformProvider() = default;

		template <typename T>
		T* Get(const std::uint32_t frameIndex) const
		{
			return static_cast<T*>(m_pContext->vkFrames[frameIndex].globalUniformBuffer->GetData());
		}

	private:
		VulkanContext* m_pContext = nullptr;
	};
};