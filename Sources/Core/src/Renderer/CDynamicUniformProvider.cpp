#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CDynamicUniformProvider::CDynamicUniformProvider(VulkanContext* pContext, const uint32 maxObjects) : m_pContext(pContext),
		m_maxObjects(maxObjects)
	{
		m_uniformAlignment = _calculateUniformAlignment();

		const uint32 count = m_pContext->vkSettings.maxFramesInFlight;
		const uint32 bufferSize = m_maxObjects * m_uniformAlignment;

		for (uint32 i = 0; i < count; i++) {
			m_pContext->vkFrames[i].dynamicUniformBuffer = CBufferManager::CreateUniformBuffer(bufferSize);
		}

		LOG_SUCCESS("Vulkan dynamic uniforms: ok (count: " << count << ")");
	}

	uint32 CDynamicUniformProvider::_calculateUniformAlignment() const
	{
		const uint32 minUboAlignment = m_pContext->vkDevice->GetMinUniformBufferOffsetAlignment();

		uint32 dynamicAlignment = sizeof(DynamicUniformData);
		if (minUboAlignment > 0)
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);

		return dynamicAlignment;
	}
}