#include "CGlobalUniformProvider.hpp"


namespace BetterThanNothing
{
	CGlobalUniformProvider::CGlobalUniformProvider(VulkanContext* pContext): m_pContext(pContext)
	{
		const uint32 count = m_pContext->vkSettings.maxFramesInFlight;
		for (uint32 i = 0; i < count; i++) {
			m_pContext->vkFrames[i].globalUniformBuffer = CBufferManager::CreateUniformBuffer<GlobalUniformData>();
		}

		LOG_SUCCESS("Vulkan global uniforms: ok (count: " << count << ")");
	}
}