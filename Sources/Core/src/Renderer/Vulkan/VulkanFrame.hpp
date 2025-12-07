//
// Created by kakesinfo on 27/11/2025.
//

#pragma once

#include <memory>

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct VulkanFrame
	{
		std::unique_ptr<CVulkanCommandBuffer>	commandBuffer;
		std::unique_ptr<CVulkanUniformBuffer>	uniformBuffer;
		std::unique_ptr<CVulkanSemaphore>		imageAvailableSemaphore;
		std::unique_ptr<CVulkanSemaphore>		renderFinishedSemaphore;
		std::unique_ptr<CVulkanFence>			inFlightFence;
	};
};
