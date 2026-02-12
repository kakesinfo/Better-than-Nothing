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

#pragma once

#include "RenderingApi/VulkanBuffer.hpp"
#include "RenderingApi/VulkanCommandBuffer.hpp"
#include "RenderingApi/VulkanCommandBufferContext.hpp"
#include "RenderingApi/VulkanCommandPool.hpp"
#include "RenderingApi/VulkanContext.hpp"
#include "RenderingApi/VulkanDescriptorPool.hpp"
#include "RenderingApi/VulkanDescriptorSet.hpp"
#include "RenderingApi/VulkanDescriptorSetLayout.hpp"
#include "RenderingApi/VulkanDevice.hpp"
#include "RenderingApi/VulkanFence.hpp"
#include "RenderingApi/VulkanFrame.hpp"
#include "RenderingApi/VulkanImage.hpp"
#include "RenderingApi/VulkanImageView.hpp"
#include "RenderingApi/VulkanIndexBuffer.hpp"
#include "RenderingApi/VulkanInstance.hpp"
#include "RenderingApi/VulkanMemoryAllocator.hpp"
#include "RenderingApi/VulkanPipeline.hpp"
#include "RenderingApi/VulkanQueue.hpp"
#include "RenderingApi/VulkanSampler.hpp"
#include "RenderingApi/VulkanSemaphore.hpp"
#include "RenderingApi/VulkanShaderModule.hpp"
#include "RenderingApi/VulkanStagingBuffer.hpp"
#include "RenderingApi/VulkanSurface.hpp"
#include "RenderingApi/VulkanStorageBuffer.hpp"
#include "RenderingApi/VulkanSwapChain.hpp"
#include "RenderingApi/VulkanUniformBuffer.hpp"
#include "RenderingApi/VulkanVertexBuffer.hpp"

#include "RenderingApi/EVulkanBufferingMode.hpp"
#include "RenderingApi/VertexInputDescription.hpp"

#include "RenderingApi/Data/ShaderGpuData.hpp"

#include "RenderingApi/Manager/BufferManager.hpp"
