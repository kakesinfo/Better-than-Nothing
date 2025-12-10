//
// Created by COEZARAX on 10/12/2025.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanShaderModule::CVulkanShaderModule(VulkanContext* vkContext, const VkShaderModuleCreateInfo* createInfo)
		: m_vkContext(vkContext)
	{
		if (vkCreateShaderModule(m_vkContext->vkDevice->Handle(), createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}

	CVulkanShaderModule::~CVulkanShaderModule()
	{
		if (m_shaderModule)
			vkDestroyShaderModule(m_vkContext->vkDevice->Handle(), m_shaderModule, nullptr);
	}

	VkShaderModule CVulkanShaderModule::Handle() const
	{
		return m_shaderModule;
	}
}
