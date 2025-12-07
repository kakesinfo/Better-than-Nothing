//
// Created by kakesinfo on 01/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct GlobalUniformData
	{
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view;
	};
}