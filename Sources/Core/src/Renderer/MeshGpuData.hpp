//
// Created by acoezard on 06/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	struct MeshGpuData
	{
		std::unique_ptr<CVulkanVertexBuffer> pVertexBuffer;
		std::unique_ptr<CVulkanIndexBuffer> pIndexBuffer;
		uint32 indexCount = 0;
	};
}