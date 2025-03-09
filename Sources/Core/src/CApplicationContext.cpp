//
// Created by acoezard on 11/5/24.
//

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	CApplicationContext::CApplicationContext(ApplicationContextWindowParams windowParams, ApplicationContextVulkanParams vulkanParams)
		: m_windowParams(std::move(windowParams)), m_vulkanParams(std::move(vulkanParams))
	{
	}

	CApplicationContext& CApplicationContext::Initialize()
	{
		m_pWindow = std::make_unique<CWindow>(
			m_windowParams.title,
			m_windowParams.width,
			m_windowParams.height,
			m_windowParams.fullscreen,
			m_windowParams.resizable
		);

		m_pVulkanInstance = std::make_unique<CVulkanInstance>(this);
		m_pVulkanSurface = std::make_unique<CVulkanSurface>(this);
		m_pVulkanDevice = std::make_unique<CVulkanDevice>(this);

		LOG_INFO("Vendor: " << m_pVulkanDevice->GetVendorName());
		LOG_INFO("Device: " << m_pVulkanDevice->GetDeviceName());
		LOG_INFO("API Version: " << m_pVulkanDevice->GetApiVersion());

		m_pVulkanMemoryAllocator = std::make_unique<CVulkanMemoryAllocator>(this);

		LOG_INFO("Buffering mode: " << GetMaxFrameInFlightCount());
		LOG_INFO("VSync: " << (m_vulkanParams.enableVSync ? "enabled" : "disabled"));

		m_pRenderer = std::make_unique<CRenderer>(this);
		m_pRenderer->Initialize();

		m_pShaderPool = std::make_unique<CShaderPool>(m_vulkanParams.shadersFolderPath, this);

		return *this;
	}

	std::unique_ptr<CWindow>& CApplicationContext::GetWindow()
	{
		if (!m_pWindow)
			throw ApplicationContextError("CWindow is not set");
		return m_pWindow;
	}

	std::unique_ptr<CVulkanInstance>& CApplicationContext::GetVulkanInstance()
	{
		if (!m_pVulkanInstance)
			throw ApplicationContextError("Vulkan instance is not set");
		return m_pVulkanInstance;
	}

	std::unique_ptr<CVulkanSurface>& CApplicationContext::GetVulkanSurface()
	{
		if (!m_pVulkanSurface)
			throw ApplicationContextError("Vulkan surface is not set");
		return m_pVulkanSurface;
	}

	std::unique_ptr<CVulkanDevice>& CApplicationContext::GetVulkanDevice()
	{
		if (!m_pVulkanDevice)
			throw ApplicationContextError("Vulkan device is not set");
		return m_pVulkanDevice;
	}

	void CApplicationContext::SetGraphicsQueue(VkQueue graphicsQueue)
	{
		m_pGraphicsQueue = std::make_unique<CVulkanQueue>(graphicsQueue);
	}

	std::unique_ptr<CVulkanQueue>& CApplicationContext::GetGraphicsQueue()
	{
		if (!m_pGraphicsQueue)
			throw ApplicationContextError("Graphics queue is not set");
		return m_pGraphicsQueue;
	}

	void CApplicationContext::SetPresentQueue(VkQueue presentQueue)
	{
		m_pPresentQueue = std::make_unique<CVulkanQueue>(presentQueue);
	}

	std::unique_ptr<CVulkanQueue>& CApplicationContext::GetPresentQueue()
	{
		if (!m_pPresentQueue)
			throw ApplicationContextError("Presentation queue is not set");
		return m_pPresentQueue;
	}

	std::unique_ptr<CVulkanMemoryAllocator>& CApplicationContext::GetVulkanMemoryAllocator()
	{
		if (!m_pVulkanMemoryAllocator)
			throw ApplicationContextError("Vulkan memory allocator is not set");
		return m_pVulkanMemoryAllocator;
	}

	std::unique_ptr<CRenderer>& CApplicationContext::GetRenderer()
	{
		if (!m_pRenderer)
			throw ApplicationContextError("Renderer is not set");
		return m_pRenderer;
	}

	std::unique_ptr<CShaderPool>& CApplicationContext::GetShaderPool()
	{
		if (!m_pShaderPool)
			throw ApplicationContextError("Shader pool is not set");
		return m_pShaderPool;
	}

	CVulkanShaderModule CApplicationContext::LoadShader(const std::string& name, EVulkanShaderType type)
	{
		return m_pShaderPool->LoadShader(name, type);
	}

	[[nodiscard]]
	uint32 CApplicationContext::GetVulkanApiVersion() const
	{
		return m_vulkanParams.apiVersion;
	}

	void CApplicationContext::EnableValidationLayers(const bool enable)
	{
		m_vulkanParams.enableValidationLayers = enable;
	}

	[[nodiscard]]
	bool CApplicationContext::IsValidationLayersEnabled() const
	{
		return m_vulkanParams.enableValidationLayers;
	}

	[[nodiscard]]
	std::span<const char* const> CApplicationContext::GetValidationLayers() const
	{
		return m_vulkanParams.validationLayers;
	}

	[[nodiscard]]
	std::span<const char* const> CApplicationContext::GetDeviceExtensions() const
	{
		return m_vulkanParams.deviceExtensions;
	}

	[[nodiscard]]
	uint32 CApplicationContext::GetMaxFrameInFlightCount() const
	{
		return static_cast<uint32>(m_vulkanParams.bufferingMode);
	}

	[[nodiscard]]
	bool CApplicationContext::IsVSyncEnabled() const
	{
		return m_vulkanParams.enableVSync;
	}
} // BetterThanNothing