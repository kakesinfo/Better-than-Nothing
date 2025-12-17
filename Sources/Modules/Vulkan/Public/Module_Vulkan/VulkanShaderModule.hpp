#pragma once

#include "EVulkanShaderType.hpp"

namespace BetterThanNothing
{
	struct VulkanShaderModule
	{
		std::string shaderPath;
		EVulkanShaderType shaderType;
		VkShaderModule shaderModule = VK_NULL_HANDLE;
	};
};
