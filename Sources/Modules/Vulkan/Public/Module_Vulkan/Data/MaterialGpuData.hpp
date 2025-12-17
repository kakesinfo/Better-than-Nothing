#pragma once

namespace BetterThanNothing
{
	class CVulkanDescriptorSet;

	struct MaterialGpuData
	{
		// TODO: add textures when supported
		std::unique_ptr<CVulkanDescriptorSet> pDescriptorSet;
	};
}