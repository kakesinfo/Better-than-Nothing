#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanFence::CVulkanFence(CVulkanContext* context): m_context(context)
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto device = m_context->vkDevice->Handle();
		if (vkCreateFence(device, &fenceInfo, nullptr, &m_fence) != VK_SUCCESS)
			throw std::runtime_error("failed to create fence!");
	}

	CVulkanFence::~CVulkanFence()
	{
		if (m_fence != VK_NULL_HANDLE)
		{
			auto device = m_context->vkDevice->Handle();
			vkDestroyFence(device, m_fence, nullptr);
		}
	}

	void CVulkanFence::Wait()
	{
		auto device = m_context->vkDevice->Handle();
		vkWaitForFences(device, 1, &m_fence, VK_TRUE, UINT64_MAX);
	}

	void CVulkanFence::Reset()
	{
		auto device = m_context->vkDevice->Handle();
		vkResetFences(device, 1, &m_fence);
	}

	VkFence CVulkanFence::Handle() const
	{
		return m_fence;
	}
}