/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include <memory>

#include "Module_Base/EBufferingMode.hpp"
#include "RenderingApi/Core/Allocator.hpp"
#include "RenderingApi/Core/CommandBuffer.hpp"
#include "RenderingApi/Core/CommandPool.hpp"
#include "RenderingApi/Core/DescriptorPool.hpp"
#include "RenderingApi/Core/Device.hpp"
#include "RenderingApi/Core/Fence.hpp"
#include "RenderingApi/Core/Instance.hpp"
#include "RenderingApi/Core/Pipeline.hpp"
#include "RenderingApi/Core/Queue.hpp"
#include "RenderingApi/Core/Semaphore.hpp"
#include "RenderingApi/Core/Surface.hpp"
#include "RenderingApi/Core/SwapChain.hpp"
#include "RenderingApi/Data/ShaderGpuData.hpp"
#include "RenderingApi/Data/VertexInputDescription.hpp"
#include "RenderingApi/Deprecated/VulkanBuffer.hpp"
#include "RenderingApi/Deprecated/VulkanContext.hpp"
#include "RenderingApi/Deprecated/VulkanDescriptorSet.hpp"
#include "RenderingApi/Deprecated/VulkanShaderModule.hpp"
#include "RenderingApi/Deprecated/VulkanStorageBuffer.hpp"
#include "RenderingApi/Deprecated/VulkanUniformBuffer.hpp"
#include <RenderingApi/Core/DescriptorSetLayout.hpp>
