//
// Created by acoezard on 11/5/24.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{

	class ApplicationContextError : public std::runtime_error
	{
	public:
		explicit ApplicationContextError(const std::string& message)
			: std::runtime_error(message) {}
	};

	struct ApplicationContextWindowParams
	{
		std::string title;
		uint32 width = 0;
		uint32 height = 0;
		bool fullscreen = false;
		bool resizable = false;
	};

	struct ApplicationContextVulkanParams
	{
		uint32 apiVersion = VK_API_VERSION_1_3;

		bool enableValidationLayers = false;
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		std::string shadersFolderPath;

		EVulkanBufferingMode bufferingMode = EVulkanBufferingMode::None;
		bool enableVSync = true;
	};

	class CApplicationContext
	{
	private:
		ApplicationContextWindowParams m_windowParams{};
		ApplicationContextVulkanParams m_vulkanParams{};

		std::unique_ptr<CWindow> m_pWindow = nullptr;

		std::unique_ptr<CVulkanInstance> m_pVulkanInstance = nullptr;
		std::unique_ptr<CVulkanSurface> m_pVulkanSurface = nullptr;
		std::unique_ptr<CVulkanDevice> m_pVulkanDevice = nullptr;
		std::unique_ptr<CVulkanQueue> m_pGraphicsQueue = nullptr;
		std::unique_ptr<CVulkanQueue> m_pPresentQueue = nullptr;
		std::unique_ptr<CVulkanMemoryAllocator> m_pVulkanMemoryAllocator = nullptr;

		std::unique_ptr<CRenderer> m_pRenderer = nullptr;
		std::unique_ptr<CShaderPool> m_pShaderPool = nullptr;

	public:
		explicit CApplicationContext(ApplicationContextWindowParams  windowParams, ApplicationContextVulkanParams  vulkanParams);

		CApplicationContext& Initialize();

		std::unique_ptr<CWindow>& GetWindow();

		std::unique_ptr<CVulkanInstance>& GetVulkanInstance();

		std::unique_ptr<CVulkanSurface>& GetVulkanSurface();

		std::unique_ptr<CVulkanDevice>& GetVulkanDevice();

		void SetGraphicsQueue(VkQueue graphicsQueue);

		std::unique_ptr<CVulkanQueue>& GetGraphicsQueue();

		void SetPresentQueue(VkQueue presentQueue);

		std::unique_ptr<CVulkanQueue>& GetPresentQueue();

		std::unique_ptr<CVulkanMemoryAllocator>& GetVulkanMemoryAllocator();

		std::unique_ptr<CRenderer>& GetRenderer();

		std::unique_ptr<CShaderPool>& GetShaderPool();

		CVulkanShaderModule LoadShader(const std::string& name, EVulkanShaderType type);

		[[nodiscard]]
		uint32 GetVulkanApiVersion() const;

		void EnableValidationLayers(bool enable);

		[[nodiscard]]
		bool IsValidationLayersEnabled() const;

		[[nodiscard]]
		std::span<const char* const> GetValidationLayers() const;

		[[nodiscard]]
		std::span<const char* const> GetDeviceExtensions() const;

		[[nodiscard]]
		uint32 GetMaxFrameInFlightCount() const;

		[[nodiscard]]
		bool IsVSyncEnabled() const;
	};

	class ApplicationContextBuilder
	{
	private:
		ApplicationContextWindowParams m_windowParams{};
		ApplicationContextVulkanParams m_vulkanParams{};

	public:
		ApplicationContextBuilder() = default;
		~ApplicationContextBuilder() = default;

		ApplicationContextBuilder& SetWindowTitle(const std::string& title)
		{
			m_windowParams.title = title;
			return *this;
		}

		ApplicationContextBuilder& SetWindowSize(uint32 width, uint32 height)
		{
			m_windowParams.width = width;
			m_windowParams.height = height;
			return *this;
		}

		ApplicationContextBuilder& SetWindowFullscreen(bool fullscreen)
		{
			m_windowParams.fullscreen = fullscreen;
			return *this;
		}

		ApplicationContextBuilder& SetWindowResizable(bool resizable)
		{
			m_windowParams.resizable = resizable;
			return *this;
		}

		ApplicationContextBuilder& SetVulkanApiVersion(uint32 version)
		{
			m_vulkanParams.apiVersion = version;
			return *this;
		}

		ApplicationContextBuilder& EnableValidationLayers(bool enable)
		{
			m_vulkanParams.enableValidationLayers = enable;
			return *this;
		}

		ApplicationContextBuilder& AddValidationLayer(const char* layer)
		{
			m_vulkanParams.validationLayers.push_back(layer);
			return *this;
		}

		ApplicationContextBuilder& RequireDeviceExtension(const char* extension)
		{
			m_vulkanParams.deviceExtensions.push_back(extension);
			return *this;
		}

		ApplicationContextBuilder& SetShadersFolderPath(const std::string& path)
		{
			m_vulkanParams.shadersFolderPath = path;
			return *this;
		}

		ApplicationContextBuilder& SetBufferingMode(EVulkanBufferingMode type)
		{
			m_vulkanParams.bufferingMode = type;
			return *this;
		}

		ApplicationContextBuilder& EnableVSync(bool enable)
		{
			m_vulkanParams.enableVSync = enable;
			return *this;
		}

		CApplicationContext Build()
		{
			if (m_windowParams.title.empty())
				throw ApplicationContextError("CWindow title is not set");

			if (m_windowParams.width == 0 || m_windowParams.height == 0)
				throw ApplicationContextError("CWindow size is not set");

			if (m_vulkanParams.enableValidationLayers && m_vulkanParams.validationLayers.empty())
				throw ApplicationContextError("Validation layers are enabled but none are set");

			if (m_vulkanParams.shadersFolderPath.empty())
				throw ApplicationContextError("Assets folder path is not set");

			if (m_vulkanParams.deviceExtensions.empty())
				throw ApplicationContextError("Device extensions are not set");

			if (m_vulkanParams.bufferingMode == EVulkanBufferingMode::None)
				throw ApplicationContextError("Buffering mode is not set");

			return CApplicationContext(m_windowParams, m_vulkanParams);
		}
	};
} // BetterThanNothing