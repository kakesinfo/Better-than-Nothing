#pragma once

namespace BetterThanNothing
{
	struct VulkanContext;

	class CVulkanShaderModule
	{
	public:
		CVulkanShaderModule(VulkanContext* vkContext, const VkShaderModuleCreateInfo* createInfo);
		~CVulkanShaderModule();

		[[nodiscard]]
		VkShaderModule Handle() const;

	private:
		VulkanContext* m_vkContext = nullptr;
		VkShaderModule m_shaderModule = VK_NULL_HANDLE;
	};
}
