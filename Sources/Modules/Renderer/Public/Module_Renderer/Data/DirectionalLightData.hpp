#pragma once

namespace BetterThanNothing
{
	struct DirectionalLightData
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
	};
}