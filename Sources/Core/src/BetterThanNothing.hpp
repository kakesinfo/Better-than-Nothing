#pragma once

#include "Manager/CInputManager.hpp"

// Graphics
#include "CWindow.hpp"
#include "Manager/CWindowManager.hpp"

// Vulkan
#include "Renderer/Vulkan/EVulkanBufferingMode.hpp"
#include "Renderer/Vulkan/CVulkanInstance.hpp"
#include "Renderer/Vulkan/CVulkanSurface.hpp"
#include "Renderer/Vulkan/CVulkanDevice.hpp"
#include "Renderer/Vulkan/CVulkanQueue.hpp"
#include "Renderer/Vulkan/CVulkanMemoryAllocator.hpp"
#include "Renderer/Vulkan/CVulkanImage.hpp"
#include "Renderer/Vulkan/CVulkanImageView.hpp"
#include "Renderer/Vulkan/CVulkanSampler.hpp"
#include "Renderer/Vulkan/CVulkanSwapChain.hpp"
#include "Renderer/Vulkan/VulkanShaderModule.hpp"
#include "Renderer/Vulkan/EVulkanShaderType.hpp"
#include "Renderer/Vulkan/CVulkanPipeline.hpp"
#include "Renderer/Vulkan/CVulkanCommandPool.hpp"
#include "Renderer/Vulkan/CVulkanCommandBuffer.hpp"
#include "Renderer/Vulkan/CVulkanFramebuffer.hpp"
#include "Renderer/Vulkan/CVulkanRenderPass.hpp"
#include "Renderer/Vulkan/CVulkanRenderPassContext.hpp"
#include "Renderer/Vulkan/CVulkanSemaphore.hpp"
#include "Renderer/Vulkan/CVulkanFence.hpp"
#include "Renderer/Vulkan/CVulkanBuffer.hpp"
#include "Renderer/Vulkan/CVulkanUniformBuffer.hpp"
#include "Renderer/Vulkan/CVulkanVertexBuffer.hpp"
#include "Renderer/Vulkan/CVulkanIndexBuffer.hpp"
#include "Renderer/Vulkan/CVulkanStagingBuffer.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include "Renderer/Vulkan/VulkanFrame.hpp"
#include "Renderer/Vulkan/CVulkanDescriptorSetLayout.hpp"
#include "Renderer/Vulkan/CVulkanDescriptorPool.hpp"
#include "Renderer/Vulkan/CVulkanDescriptorSet.hpp"

#include "Manager/CBufferManager.hpp"

#include "Renderer/Data/TextureData.hpp"
#include "Renderer/Data/VertexData.hpp"
#include "Renderer/Data/MeshData.hpp"
#include "Renderer/Data/ShaderData.hpp"

#include "Public/IRenderer.hpp"
#include "Public/IPipelineBuilder.hpp"

#include "Renderer/Data/GlobalUniformData.hpp"
#include "Renderer/Data/DynamicUniformData.hpp"

#include "Renderer/CGlobalUniformProvider.hpp"
#include "Renderer/CDynamicUniformProvider.hpp"

#include "Renderer/Data/TextureGpuData.hpp"
#include "Renderer/Data/MaterialGpuData.hpp"
#include "Renderer/Data/MeshGpuData.hpp"
#include "Renderer/Data/ShaderGpuData.hpp"

#include "Renderer/Resources/IResourceLoader.hpp"
#include "Renderer/Resources/CTextureLoader.hpp"
#include "Renderer/Resources/CShaderLoader.hpp"

#include "Renderer/CRenderCache.hpp"
#include "Renderer/CRenderList.hpp"
#include "Renderer/CRenderer.hpp"
#include "Renderer/CRendererBuilder.hpp"

// CApplication
#include "CWindowBuilder.hpp"
#include "CApplication.hpp"
