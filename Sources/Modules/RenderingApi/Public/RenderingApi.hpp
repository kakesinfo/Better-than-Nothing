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

#include "RenderingApi/Allocator.hpp"
#include "RenderingApi/CommandBuffer.hpp"
#include "RenderingApi/CommandPool.hpp"
#include "RenderingApi/DescriptorPool.hpp"
#include "RenderingApi/Device.hpp"
#include "RenderingApi/Fence.hpp"
#include "RenderingApi/Instance.hpp"
#include "RenderingApi/Old/VulkanBuffer.hpp"
#include "RenderingApi/Old/VulkanCommandBufferContext.hpp"
#include "RenderingApi/Old/VulkanContext.hpp"
#include "RenderingApi/Old/VulkanDescriptorSet.hpp"
#include "RenderingApi/Old/VulkanDescriptorSetLayout.hpp"
#include "RenderingApi/Old/VulkanFrame.hpp"
#include "RenderingApi/Old/VulkanImage.hpp"
#include "RenderingApi/Old/VulkanImageView.hpp"
#include "RenderingApi/Old/VulkanIndexBuffer.hpp"
#include "RenderingApi/Old/VulkanShaderModule.hpp"
#include "RenderingApi/Old/VulkanUniformBuffer.hpp"
#include "RenderingApi/Pipeline.hpp"
#include "RenderingApi/Queue.hpp"
#include "RenderingApi/Semaphore.hpp"
#include "RenderingApi/Surface.hpp"
#include "RenderingApi/SwapChain.hpp"
#include "RenderingApi/Old/VulkanSampler.hpp"
#include "RenderingApi/Old/VulkanStagingBuffer.hpp"
#include "RenderingApi/Old/VulkanStorageBuffer.hpp"
#include "RenderingApi/Old/VulkanVertexBuffer.hpp"

#include "EVulkanBufferingMode.hpp"
#include "RenderingApi/Old/VertexInputDescription.hpp"

#include "RenderingApi/Data/ShaderGpuData.hpp"

#include "RenderingApi/Manager/BufferManager.hpp"
