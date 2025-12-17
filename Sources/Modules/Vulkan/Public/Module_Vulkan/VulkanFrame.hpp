#pragma once

namespace BetterThanNothing
{
	class CVulkanCommandBuffer;
	class CVulkanUniformBuffer;
	class CVulkanSemaphore;
	class CVulkanFence;

	struct VulkanFrame
	{
		std::unique_ptr<CVulkanCommandBuffer>	commandBuffer;
		std::unique_ptr<CVulkanUniformBuffer>	globalUniformBuffer;
		std::unique_ptr<CVulkanUniformBuffer>	dynamicUniformBuffer;
		std::unique_ptr<CVulkanSemaphore>		imageAvailableSemaphore;
		std::unique_ptr<CVulkanSemaphore>		renderFinishedSemaphore;
		std::unique_ptr<CVulkanFence>			inFlightFence;
	};
};
