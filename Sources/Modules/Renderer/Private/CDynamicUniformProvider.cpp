#include "module_renderer.hpp"
#include <Module_Base/CLogger.hpp>

namespace BetterThanNothing
{
	CDynamicUniformProvider::CDynamicUniformProvider(VulkanContext* pContext, const std::uint32_t maxObjects) : m_pContext(pContext),
		m_maxObjects(maxObjects)
	{
		m_uniformAlignment = _calculateUniformAlignment();

		const std::uint32_t count = m_pContext->vkSettings.maxFramesInFlight;
		const std::uint32_t bufferSize = m_maxObjects * m_uniformAlignment;

		for (std::uint32_t i = 0; i < count; i++) {
			m_pContext->vkFrames[i].dynamicUniformBuffer = CBufferManager::CreateUniformBuffer(bufferSize);
		}

		CLogger::Log<Success>("Vulkan dynamic uniforms: ok (count: {})", count);
	}

	std::uint32_t CDynamicUniformProvider::_calculateUniformAlignment() const
	{
		const std::uint32_t minUboAlignment = m_pContext->vkDevice->GetMinUniformBufferOffsetAlignment();

		std::uint32_t dynamicAlignment = sizeof(DynamicUniformData);
		if (minUboAlignment > 0)
			dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);

		return dynamicAlignment;
	}
}
