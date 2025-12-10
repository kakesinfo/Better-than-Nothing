//
// Created by COEZARAX on 09/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct ShaderGpuData
	{
		EVulkanShaderType						type{};
		std::unique_ptr<CVulkanShaderModule>	pVkShaderModule = VK_NULL_HANDLE;
	};
};