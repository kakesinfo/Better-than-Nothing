//
// Created by COEZARAX on 08/12/2025.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanSampler::CVulkanSampler(VulkanContext* context, const VkSamplerCreateInfo* samplerCreateInfo) : m_context(context)
	{
		if (vkCreateSampler(m_context->vkDevice->Handle(), samplerCreateInfo, nullptr, &m_sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}

	CVulkanSampler::~CVulkanSampler()
	{
		if (m_sampler)
			vkDestroySampler(m_context->vkDevice->Handle(), m_sampler, nullptr);
	}

	VkSampler CVulkanSampler::Handle() const
	{
		return m_sampler;
	}
}