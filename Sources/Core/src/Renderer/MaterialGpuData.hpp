//
// Created by acoezard on 06/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct MaterialGpuData
	{
		// TODO: add textures when supported
		std::unique_ptr<CVulkanDescriptorSet> pDescriptorSet;
	};
}