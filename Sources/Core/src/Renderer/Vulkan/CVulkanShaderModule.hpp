#pragma once

#include "EVulkanShaderType.hpp"

namespace BetterThanNothing
{
	struct CVulkanShaderModule
	{
		std::string shaderPath;
		EVulkanShaderType shaderType;
		VkShaderModule shaderModule = VK_NULL_HANDLE;
	};
};
