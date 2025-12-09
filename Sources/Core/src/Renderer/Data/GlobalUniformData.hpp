//
// Created by kakesinfo on 01/12/2025.
//

#pragma once

namespace BetterThanNothing
{
	struct GlobalUniformData
	{
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view;
	};
}