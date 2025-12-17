#include "module_renderer.hpp"
#include <Module_Base/CLogger.hpp>

namespace BetterThanNothing
{
	CGlobalUniformProvider::CGlobalUniformProvider(VulkanContext* pContext): m_pContext(pContext)
	{
		const std::uint32_t count = m_pContext->vkSettings.maxFramesInFlight;
		for (std::uint32_t i = 0; i < count; i++) {
			m_pContext->vkFrames[i].globalUniformBuffer = CBufferManager::CreateUniformBuffer<GlobalUniformData>();
		}

		CLogger::Log<Success>("Vulkan global uniforms: ok (count: {})", count);
	}
}
