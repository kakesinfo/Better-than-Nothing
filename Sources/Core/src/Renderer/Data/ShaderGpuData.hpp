//
// Created by COEZARAX on 09/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct ShaderGpuData
	{
		EVulkanShaderType	type{};
		VkShaderModule		vkModule = VK_NULL_HANDLE; // TODO: use unique_ptr
	};
};