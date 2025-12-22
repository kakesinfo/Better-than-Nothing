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

#include "Module_Vulkan/VulkanBuffer.hpp"
#include "Module_Vulkan/VulkanCommandBuffer.hpp"
#include "Module_Vulkan/VulkanCommandBufferContext.hpp"
#include "Module_Vulkan/VulkanCommandPool.hpp"
#include "Module_Vulkan/VulkanContext.hpp"
#include "Module_Vulkan/VulkanDescriptorPool.hpp"
#include "Module_Vulkan/VulkanDescriptorSet.hpp"
#include "Module_Vulkan/VulkanDescriptorSetLayout.hpp"
#include "Module_Vulkan/VulkanDevice.hpp"
#include "Module_Vulkan/VulkanFence.hpp"
#include "Module_Vulkan/VulkanFrame.hpp"
#include "Module_Vulkan/VulkanImage.hpp"
#include "Module_Vulkan/VulkanImageView.hpp"
#include "Module_Vulkan/VulkanIndexBuffer.hpp"
#include "Module_Vulkan/VulkanInstance.hpp"
#include "Module_Vulkan/VulkanMemoryAllocator.hpp"
#include "Module_Vulkan/VulkanPipeline.hpp"
#include "Module_Vulkan/VulkanQueue.hpp"
#include "Module_Vulkan/VulkanSampler.hpp"
#include "Module_Vulkan/VulkanSemaphore.hpp"
#include "Module_Vulkan/VulkanShaderModule.hpp"
#include "Module_Vulkan/VulkanStagingBuffer.hpp"
#include "Module_Vulkan/VulkanSurface.hpp"
#include "Module_Vulkan/VulkanSwapChain.hpp"
#include "Module_Vulkan/VulkanUniformBuffer.hpp"
#include "Module_Vulkan/VulkanVertexBuffer.hpp"

#include "Module_Vulkan/EVulkanBufferingMode.hpp"
#include "Module_Vulkan/EVulkanShaderType.hpp"
#include "Module_Vulkan/VertexInputDescription.hpp"

#include "Module_Vulkan/Data/MaterialGpuData.hpp"
#include "Module_Vulkan/Data/MeshGpuData.hpp"
#include "Module_Vulkan/Data/ShaderGpuData.hpp"
#include "Module_Vulkan/Data/TextureGpuData.hpp"

#include "Module_Vulkan/Manager/BufferManager.hpp"
