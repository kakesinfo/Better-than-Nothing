//
// Created by kakesinfo on 27/11/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
    class CRendererBuilderError final : public std::runtime_error {
    public:
        explicit CRendererBuilderError(const std::string& message)
            : std::runtime_error(message) {}
    };

    class CRendererBuilder
    {
    private:
        uint32 apiVersion = VK_API_VERSION_1_3;

        bool enableValidationLayers = false;
        std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME };

        std::string shadersFolderPath;

        EVulkanBufferingMode bufferingMode = EVulkanBufferingMode::None;
        bool enableVSync = true;

    public:
        CRendererBuilder() = default;
        ~CRendererBuilder() = default;

        CRendererBuilder& SetVulkanApiVersion(const uint32_t version)
        {
            apiVersion = version;
            return *this;
        }

        CRendererBuilder& EnableValidationLayers(const bool enable)
        {
            enableValidationLayers = enable;
            return *this;
        }

        CRendererBuilder& AddValidationLayer(const char* layer)
        {
            validationLayers.push_back(layer);
            return *this;
        }

        CRendererBuilder& RequireDeviceExtension(const char* extension)
        {
            deviceExtensions.push_back(extension);
            return *this;
        }

        CRendererBuilder& SetShadersFolderPath(const std::string& path)
        {
            shadersFolderPath = path;
            return *this;
        }

        CRendererBuilder& SetBufferingMode(const EVulkanBufferingMode type)
        {
            bufferingMode = type;
            return *this;
        }

        CRendererBuilder& EnableVSync(const bool enable)
        {
            enableVSync = enable;
            return *this;
        }

        std::unique_ptr<CRenderer> Build()
        {
            if (enableValidationLayers && validationLayers.empty())
                throw CRendererBuilderError("Validation layers are enabled but none are set");

            if (shadersFolderPath.empty())
                throw CRendererBuilderError("Assets folder path is not set");

            if (deviceExtensions.empty())
                throw CRendererBuilderError("Device extensions are not set");

            if (bufferingMode == EVulkanBufferingMode::None)
                throw CRendererBuilderError("Buffering mode is not set");

            CVulkanContext vulkanContext;
            vulkanContext.vkSettings.apiVersion = apiVersion;
            vulkanContext.vkSettings.enableValidationLayers = enableValidationLayers;
            vulkanContext.vkSettings.validationLayers = validationLayers;
            vulkanContext.vkSettings.deviceExtensions = deviceExtensions;
            vulkanContext.vkSettings.shadersFolderPath = shadersFolderPath;
            vulkanContext.vkSettings.bufferingMode = bufferingMode;
            vulkanContext.vkSettings.maxFramesInFlight = static_cast<uint32>(bufferingMode);
            vulkanContext.vkSettings.enableVSync = enableVSync;

            return std::make_unique<CRenderer>(vulkanContext);
        }
    };
} // BetterThanNothing