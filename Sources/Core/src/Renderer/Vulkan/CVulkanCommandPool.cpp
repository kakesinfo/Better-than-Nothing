#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CVulkanCommandPool::CVulkanCommandPool(VulkanContext* context): m_context(context)
	{
		auto device = m_context->vkDevice->Handle();
		auto queueFamilyIndices = m_context->vkDevice->GetQueueFamilies();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");

		LOG_SUCCESS("Vulkan command pool: ok");
	}

	CVulkanCommandPool::~CVulkanCommandPool()
	{
		vkDestroyCommandPool(m_context->vkDevice->Handle(), m_commandPool, nullptr);
	}

	VkCommandPool CVulkanCommandPool::Handle() const
	{
		return m_commandPool;
	}
}