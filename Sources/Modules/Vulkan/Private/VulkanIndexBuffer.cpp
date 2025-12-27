/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "module_vulkan.hpp"

namespace BetterThanNothing
{
	VulkanIndexBuffer::VulkanIndexBuffer(VulkanContext* context, VkDeviceSize size, const void* data)
		: VulkanBuffer(context, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					   VMA_MEMORY_USAGE_GPU_ONLY, 0)
	{
		if (data)
		{
			const VulkanStagingBuffer stagingBuffer(context, size, data);

			VulkanCommandBuffer::SingleTimeCommands(
				[&](const std::unique_ptr<VulkanCommandBuffer>& commandBuffer) {
					commandBuffer->CopyBuffer(stagingBuffer.Handle(), m_buffer, size);
				},
				context);
		}
	}
}
