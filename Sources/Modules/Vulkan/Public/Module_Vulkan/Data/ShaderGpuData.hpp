//
// Created by COEZARAX on 09/12/2025.
//

#pragma once

#include "../EVulkanShaderType.hpp"

namespace BetterThanNothing
{
	class CVulkanShaderModule;

	struct ShaderGpuData
	{
		EVulkanShaderType						type{};
		std::unique_ptr<CVulkanShaderModule>	pVkShaderModule = VK_NULL_HANDLE;
	};
};