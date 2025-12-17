#pragma once

namespace BetterThanNothing
{
	class CVulkanVertexBuffer;
	class CVulkanIndexBuffer;

	struct MeshGpuData
	{
		std::unique_ptr<CVulkanVertexBuffer> pVertexBuffer;
		std::unique_ptr<CVulkanIndexBuffer> pIndexBuffer;
		std::uint32_t indexCount = 0;
	};
}