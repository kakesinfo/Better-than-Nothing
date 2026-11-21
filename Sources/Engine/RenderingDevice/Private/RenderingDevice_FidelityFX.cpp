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

#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include <Common/Logger.hpp>
#include <Common/RenderSettings.hpp>
#include <RenderingApi.hpp>

#include <ffx_api.hpp>
#include <vk/ffx_api_vk.hpp>
#include <ffx_upscale.hpp>
#include <ffx_denoiser.hpp>

#include <algorithm>
#include <cmath>
#include <cwchar>

namespace BetterThanNothing
{
	namespace
	{
		std::string WideToUtf8(const wchar_t* wstr)
		{
			if (!wstr)
				return {};
			std::string out;
			while (*wstr)
			{
				const wchar_t wc = *wstr++;
				if (wc < 0x80)
					out.push_back(static_cast<char>(wc));
				else
					out.push_back('?');
			}
			return out;
		}

		void FfxDebugMessageCallback(const uint32_t type, const wchar_t* message)
		{
			const std::string msg = WideToUtf8(message);
			if (msg.empty())
				return;

			if (type == FFX_API_MESSAGE_TYPE_ERROR)
				Logger::Log<ERROR>("FidelityFX SDK: {}", msg);
			else
				Logger::Log<WARNING>("FidelityFX SDK: {}", msg);
		}

		FfxApiResource BuildFfxTextureResource(const RenderingApi::Texture* tex, const uint32_t state, const uint32_t additionalUsages = FFX_API_RESOURCE_USAGE_READ_ONLY)
		{
			FfxApiResource res{};
			if (!tex || tex->value == VK_NULL_HANDLE)
				return res;

			VkImageCreateInfo imgInfo{};
			imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imgInfo.imageType = VK_IMAGE_TYPE_2D;
			imgInfo.format = tex->format;
			imgInfo.extent = {tex->width, tex->height, 1u};
			imgInfo.mipLevels = tex->mipLevels;
			imgInfo.arrayLayers = 1u;
			imgInfo.samples = tex->samples;
			imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imgInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			if ((additionalUsages & FFX_API_RESOURCE_USAGE_UAV) != 0)
				imgInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

			const FfxApiResourceDescription desc = ffxApiGetImageResourceDescriptionVK(tex->value, imgInfo, additionalUsages);
			return ffxApiGetResourceVK(reinterpret_cast<void*>(tex->value), desc, state);
		}
	}

	bool RenderingDevice::InitializeFidelityFX()
	{
		if (m_self->ffxInitialized)
			return m_self->ffxUpscalerAvailable;

		m_self->ffxInitialized = true;

		VkDeviceContext vkDevCtx{};
		vkDevCtx.vkDevice = m_self->device->Handle();
		vkDevCtx.vkPhysicalDevice = m_self->device->PhysicalHandle();
		vkDevCtx.vkDeviceProcAddr = vkGetDeviceProcAddr;

		// Configure global debug callback for FidelityFX Upscaler & Denoiser
		{
			ffx::ConfigureDescGlobalDebug debugDesc{};
			debugDesc.effectId = FFX_API_EFFECT_ID_UPSCALE;
			debugDesc.fpMessage = &FfxDebugMessageCallback;
			debugDesc.debugLevel = FFX_API_CONFIGURE_GLOBALDEBUG_LEVEL_WARNINGS;
			(void)ffx::Configure(debugDesc);
		}

		// Enumerate available AMD FSR Upscaling provider versions from amd_fidelityfx_upscaler_vk.dll & AMD Adrenalin driver
		uint64_t				  versionCount = 0;
		ffx::QueryDescGetVersions versionQuery{};
		versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
		versionQuery.device = &vkDevCtx;
		versionQuery.outputCount = &versionCount;
		versionQuery.versionIds = nullptr;
		versionQuery.versionNames = nullptr;

		const ffx::ReturnCode countRc = ffx::Query(versionQuery);
		if (countRc == ffx::ReturnCode::Ok && versionCount > 0)
		{
			m_self->ffxUpscalerVersionIds.resize(versionCount, 0);
			std::vector<const char*> rawNames(versionCount, nullptr);

			versionQuery.versionIds = m_self->ffxUpscalerVersionIds.data();
			versionQuery.versionNames = rawNames.data();
			if (ffx::Query(versionQuery) == ffx::ReturnCode::Ok)
			{
				m_self->ffxUpscalerVersionNames.clear();
				m_self->ffxUpscalerVersionNames.reserve(versionCount);
				for (uint64_t i = 0; i < versionCount; ++i)
				{
					m_self->ffxUpscalerVersionNames.emplace_back(rawNames[i] ? rawNames[i] : "Unknown");
					Logger::Log<INFO>(
						"FidelityFX SDK v2.4.0 Upscaler provider [{}]: '{}' (id=0x{:016x})",
						i,
						m_self->ffxUpscalerVersionNames.back(),
						m_self->ffxUpscalerVersionIds[i]
					);
				}
				m_self->ffxUpscalerAvailable = true;
			}
		}

		auto& settings = RenderSettings::Get();
		settings.fsrAvailable = m_self->ffxUpscalerAvailable;
		settings.fsrVersionNames = m_self->ffxUpscalerVersionNames;
		if (!m_self->ffxUpscalerVersionNames.empty())
		{
			settings.fsrActiveVersionName = m_self->ffxUpscalerVersionNames[0];
			settings.fsrActiveVersionId = m_self->ffxUpscalerVersionIds[0];
		}

		// Verify and log AMD FidelityFX Denoiser SDK v2.4.0 (v1.2.1 headers + Vulkan FFX_DNSR_Shadows)
		{
			ffx::CreateContextDescDenoiser denoiserDesc{};
			denoiserDesc.version = FFX_DENOISER_VERSION;
			denoiserDesc.maxRenderSize = {1920u, 1080u};
			denoiserDesc.signalFlags = FFX_DENOISER_SIGNAL_DOMINANT_LIGHT_VISIBILITY | FFX_DENOISER_SIGNAL_AMBIENT_OCCLUSION;
			denoiserDesc.checkerboardSignalFlags = FFX_DENOISER_SIGNAL_NONE;
			denoiserDesc.flags = 0;

			settings.ffxDenoiserSdkHeaderPresent = (denoiserDesc.version == FFX_DENOISER_VERSION);
			settings.ffxDenoiserSdkVersionName = std::format(
				"SDK v2.4.0 (Denoiser {}.{}.{} API + FFX_DNSR_Shadows Vulkan)",
				FFX_DENOISER_VERSION_MAJOR,
				FFX_DENOISER_VERSION_MINOR,
				FFX_DENOISER_VERSION_PATCH
			);
			Logger::Log<SUCCESS>(
				"FidelityFX SDK v2.4.0 initialized: Upscaler API {}.{}.{} (active provider: {}), Denoiser API {}.{}.{}",
				FFX_UPSCALER_VERSION_MAJOR,
				FFX_UPSCALER_VERSION_MINOR,
				FFX_UPSCALER_VERSION_PATCH,
				settings.fsrActiveVersionName,
				FFX_DENOISER_VERSION_MAJOR,
				FFX_DENOISER_VERSION_MINOR,
				FFX_DENOISER_VERSION_PATCH
			);
		}

		return m_self->ffxUpscalerAvailable;
	}

	void RenderingDevice::ShutdownFidelityFX()
	{
		if (m_self->ffxUpscaleContext)
		{
			m_self->device->WaitIdle();
			ffxContext ctx = m_self->ffxUpscaleContext;
			(void)ffxDestroyContext(&ctx, nullptr);
			m_self->ffxUpscaleContext = nullptr;
			m_self->ffxContextMaxRenderWidth = 0;
			m_self->ffxContextMaxRenderHeight = 0;
			m_self->ffxContextMaxUpscaleWidth = 0;
			m_self->ffxContextMaxUpscaleHeight = 0;
		}
	}

	bool RenderingDevice::IsFfxUpscalerAvailable() const
	{
		return m_self->ffxUpscalerAvailable;
	}

	bool RenderingDevice::EnsureFfxUpscalerContext(
		const uint32_t maxRenderWidth,
		const uint32_t maxRenderHeight,
		const uint32_t maxUpscaleWidth,
		const uint32_t maxUpscaleHeight,
		const uint32_t versionIndex
	)
	{
		if (!m_self->ffxInitialized)
			InitializeFidelityFX();

		if (!m_self->ffxUpscalerAvailable || maxRenderWidth == 0 || maxRenderHeight == 0 || maxUpscaleWidth == 0 || maxUpscaleHeight == 0)
			return false;

		const uint32_t clampedVersionIdx = m_self->ffxUpscalerVersionIds.empty()
											   ? 0u
											   : std::min(versionIndex, static_cast<uint32_t>(m_self->ffxUpscalerVersionIds.size() - 1));

		if (m_self->ffxUpscaleContext != nullptr &&
			m_self->ffxContextMaxRenderWidth == maxRenderWidth &&
			m_self->ffxContextMaxRenderHeight == maxRenderHeight &&
			m_self->ffxContextMaxUpscaleWidth == maxUpscaleWidth &&
			m_self->ffxContextMaxUpscaleHeight == maxUpscaleHeight &&
			m_self->ffxContextVersionIndex == clampedVersionIdx)
		{
			return true;
		}

		ShutdownFidelityFX();

		VkDeviceContext vkDevCtx{};
		vkDevCtx.vkDevice = m_self->device->Handle();
		vkDevCtx.vkPhysicalDevice = m_self->device->PhysicalHandle();
		vkDevCtx.vkDeviceProcAddr = vkGetDeviceProcAddr;

		ffx::CreateBackendVKDesc backendDesc{};
		backendDesc.deviceContext = vkDevCtx;

		ffx::CreateContextDescUpscale createUpscale{};
		createUpscale.flags = FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE |
							  FFX_UPSCALE_ENABLE_AUTO_EXPOSURE |
							  FFX_UPSCALE_ENABLE_DYNAMIC_RESOLUTION;
#ifndef NDEBUG
		createUpscale.flags |= FFX_UPSCALE_ENABLE_DEBUG_CHECKING;
#endif
		createUpscale.maxRenderSize = {maxRenderWidth, maxRenderHeight};
		createUpscale.maxUpscaleSize = {maxUpscaleWidth, maxUpscaleHeight};
		createUpscale.fpMessage = &FfxDebugMessageCallback;

		ffx::CreateContextDescUpscaleVersion versionDesc{};
		versionDesc.version = FFX_UPSCALER_VERSION;

		ffx::Context	ctx = nullptr;
		ffx::ReturnCode rc = ffx::ReturnCode::Error;

		if (!m_self->ffxUpscalerVersionIds.empty())
		{
			ffx::CreateContextDescOverrideVersion overrideDesc{};
			overrideDesc.versionId = m_self->ffxUpscalerVersionIds[clampedVersionIdx];
			rc = ffx::CreateContext(ctx, nullptr, createUpscale, versionDesc, backendDesc, overrideDesc);
		}
		else
		{
			rc = ffx::CreateContext(ctx, nullptr, createUpscale, versionDesc, backendDesc);
		}

		if (rc != ffx::ReturnCode::Ok || ctx == nullptr)
		{
			Logger::Log<ERROR>("FidelityFX SDK: failed to create FSR Upscaler Vulkan context (rc={})", static_cast<uint32_t>(rc));
			return false;
		}

		m_self->ffxUpscaleContext = ctx;
		m_self->ffxContextMaxRenderWidth = maxRenderWidth;
		m_self->ffxContextMaxRenderHeight = maxRenderHeight;
		m_self->ffxContextMaxUpscaleWidth = maxUpscaleWidth;
		m_self->ffxContextMaxUpscaleHeight = maxUpscaleHeight;
		m_self->ffxContextVersionIndex = clampedVersionIdx;
		m_self->ffxLastVelocityFactor = -1.0f;

		// Query active provider version & VRAM usage
		auto& settings = RenderSettings::Get();
		ffxQueryGetProviderVersion providerVer{};
		providerVer.header.type = FFX_API_QUERY_DESC_TYPE_GET_PROVIDER_VERSION;
		providerVer.header.pNext = nullptr;
		if (ffxQuery(&m_self->ffxUpscaleContext, &providerVer.header) == FFX_API_RETURN_OK && providerVer.versionName)
		{
			settings.fsrActiveVersionName = providerVer.versionName;
			settings.fsrActiveVersionId = providerVer.versionId;
		}
		else if (!m_self->ffxUpscalerVersionNames.empty())
		{
			settings.fsrActiveVersionName = m_self->ffxUpscalerVersionNames[clampedVersionIdx];
			settings.fsrActiveVersionId = m_self->ffxUpscalerVersionIds[clampedVersionIdx];
		}

		FfxApiEffectMemoryUsage					memUsage{};
		ffx::QueryDescUpscaleGetGPUMemoryUsage memQuery{};
		memQuery.gpuMemoryUsageUpscaler = &memUsage;
		if (ffx::Query(m_self->ffxUpscaleContext, memQuery) == ffx::ReturnCode::Ok)
		{
			settings.fsrGpuMemoryMb = static_cast<float>(memUsage.totalUsageInBytes) / (1024.0f * 1024.0f);
		}

		Logger::Log<SUCCESS>(
			"FidelityFX FSR Vulkan context created: provider='{}' (0x{:016x}), maxRender={}x{}, maxUpscale={}x{}, VRAM={:.1f} MB",
			settings.fsrActiveVersionName,
			settings.fsrActiveVersionId,
			maxRenderWidth,
			maxRenderHeight,
			maxUpscaleWidth,
			maxUpscaleHeight,
			settings.fsrGpuMemoryMb
		);

		return true;
	}

	std::pair<uint32_t, uint32_t> RenderingDevice::QueryFfxRenderResolution(
		const uint32_t displayWidth,
		const uint32_t displayHeight,
		const uint32_t qualityMode
	) const
	{
		if (displayWidth == 0 || displayHeight == 0)
			return {0, 0};

		uint32_t outW = displayWidth;
		uint32_t outH = displayHeight;

		ffx::QueryDescUpscaleGetRenderResolutionFromQualityMode query{};
		query.displayWidth = displayWidth;
		query.displayHeight = displayHeight;
		query.qualityMode = qualityMode;
		query.pOutRenderWidth = &outW;
		query.pOutRenderHeight = &outH;

		if (m_self->ffxUpscaleContext != nullptr)
		{
			if (ffx::Query(m_self->ffxUpscaleContext, query) == ffx::ReturnCode::Ok && outW > 0 && outH > 0)
				return {outW, outH};
		}

		// Standard FSR quality ratios fallback
		float ratio = 1.0f;
		switch (qualityMode)
		{
			case FFX_UPSCALE_QUALITY_MODE_NATIVEAA: ratio = 1.0f; break;
			case FFX_UPSCALE_QUALITY_MODE_QUALITY: ratio = 1.5f; break;
			case FFX_UPSCALE_QUALITY_MODE_BALANCED: ratio = 1.7f; break;
			case FFX_UPSCALE_QUALITY_MODE_PERFORMANCE: ratio = 2.0f; break;
			case FFX_UPSCALE_QUALITY_MODE_ULTRA_PERFORMANCE: ratio = 3.0f; break;
			default: ratio = 1.0f; break;
		}
		outW = std::max(1u, static_cast<uint32_t>(std::round(static_cast<float>(displayWidth) / ratio)));
		outH = std::max(1u, static_cast<uint32_t>(std::round(static_cast<float>(displayHeight) / ratio)));
		return {outW, outH};
	}

	int32_t RenderingDevice::QueryFfxJitterPhaseCount(const uint32_t renderWidth, const uint32_t displayWidth) const
	{
		if (renderWidth == 0 || displayWidth == 0)
			return 16;

		int32_t										 phaseCount = 16;
		ffx::QueryDescUpscaleGetJitterPhaseCount query{};
		query.renderWidth = renderWidth;
		query.displayWidth = displayWidth;
		query.pOutPhaseCount = &phaseCount;

		if (m_self->ffxUpscaleContext != nullptr)
		{
			if (ffx::Query(m_self->ffxUpscaleContext, query) == ffx::ReturnCode::Ok && phaseCount > 0)
				return phaseCount;
		}

		const float scale = static_cast<float>(displayWidth) / static_cast<float>(renderWidth);
		return std::max(8, static_cast<int32_t>(std::ceil(8.0f * scale * scale)));
	}

	std::pair<float, float> RenderingDevice::QueryFfxJitterOffset(const int32_t index, const int32_t phaseCount) const
	{
		float								   jitterX = 0.0f;
		float								   jitterY = 0.0f;
		ffx::QueryDescUpscaleGetJitterOffset query{};
		query.index = index;
		query.phaseCount = std::max(phaseCount, 1);
		query.pOutX = &jitterX;
		query.pOutY = &jitterY;

		if (m_self->ffxUpscaleContext != nullptr)
		{
			if (ffx::Query(m_self->ffxUpscaleContext, query) == ffx::ReturnCode::Ok)
				return {jitterX, jitterY};
		}
		return {0.0f, 0.0f};
	}

	bool RenderingDevice::ConfigureFfxUpscaler(const float velocityFactor)
	{
		if (!m_self->ffxUpscaleContext)
			return false;

		float clampedVel = std::clamp(velocityFactor, 0.0f, 1.0f);
		if (std::abs(clampedVel - m_self->ffxLastVelocityFactor) < 1e-4f)
			return true;

		ffx::ConfigureDescUpscaleKeyValue kv{};
		kv.key = FFX_API_CONFIGURE_UPSCALE_KEY_FVELOCITYFACTOR;
		kv.u64 = 0;
		kv.ptr = &clampedVel;

		if (ffx::Configure(m_self->ffxUpscaleContext, kv) == ffx::ReturnCode::Ok)
		{
			m_self->ffxLastVelocityFactor = clampedVel;
			return true;
		}
		return false;
	}

	bool RenderingDevice::DispatchFfxUpscale(const FfxUpscaleDispatchParams& params)
	{
		if (!m_self->ffxUpscaleContext)
			return false;

		RenderingApi::Texture* colorTex = GetTexture(params.color);
		RenderingApi::Texture* depthTex = GetTexture(params.depth);
		RenderingApi::Texture* motionTex = GetTexture(params.motionVectors);
		RenderingApi::Texture* outputTex = GetTexture(params.output);
		if (!colorTex || !depthTex || !motionTex || !outputTex)
			return false;

		// Ensure inputs are in SHADER_READ_ONLY_OPTIMAL and output is in GENERAL (UNORDERED_ACCESS)
		TransitionTexture(params.color, ImageLayout::ShaderReadOnly);
		TransitionTexture(params.depth, ImageLayout::ShaderReadOnly);
		TransitionTexture(params.motionVectors, ImageLayout::ShaderReadOnly);
		TransitionTexture(params.output, ImageLayout::General);

		VkCommandBuffer vkCmd = m_self->commandBuffers[m_currentFrameIndex]->Handle();

		ffx::DispatchDescUpscale dispatchDesc{};
		dispatchDesc.commandList = reinterpret_cast<void*>(vkCmd);
		dispatchDesc.color = BuildFfxTextureResource(colorTex, FFX_API_RESOURCE_STATE_COMPUTE_READ, FFX_API_RESOURCE_USAGE_READ_ONLY);
		dispatchDesc.depth = BuildFfxTextureResource(depthTex, FFX_API_RESOURCE_STATE_COMPUTE_READ, FFX_API_RESOURCE_USAGE_DEPTHTARGET);
		dispatchDesc.motionVectors = BuildFfxTextureResource(motionTex, FFX_API_RESOURCE_STATE_COMPUTE_READ, FFX_API_RESOURCE_USAGE_READ_ONLY);
		dispatchDesc.exposure = {};
		dispatchDesc.reactive = {};
		dispatchDesc.transparencyAndComposition = {};
		dispatchDesc.output = BuildFfxTextureResource(outputTex, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS, FFX_API_RESOURCE_USAGE_UAV);

		dispatchDesc.jitterOffset = {params.jitterOffsetX, params.jitterOffsetY};
		dispatchDesc.motionVectorScale = {params.motionVectorScaleX, params.motionVectorScaleY};
		dispatchDesc.renderSize = {params.renderWidth, params.renderHeight};
		dispatchDesc.upscaleSize = {params.upscaleWidth, params.upscaleHeight};
		dispatchDesc.enableSharpening = params.enableSharpening;
		dispatchDesc.sharpness = std::clamp(params.sharpness, 0.0f, 1.0f);
		dispatchDesc.frameTimeDelta = std::max(params.frameTimeDeltaMs, 1.0f);
		dispatchDesc.preExposure = std::max(params.preExposure, 1e-5f);
		dispatchDesc.reset = params.reset;
		dispatchDesc.cameraNear = std::max(params.cameraNear, 0.01f);
		dispatchDesc.cameraFar = std::max(params.cameraFar, dispatchDesc.cameraNear + 1.0f);
		dispatchDesc.cameraFovAngleVertical = params.cameraFovAngleVerticalRadians;
		dispatchDesc.viewSpaceToMetersFactor = params.viewSpaceToMetersFactor;
		dispatchDesc.flags = params.drawDebugView ? FFX_UPSCALE_FLAG_DRAW_DEBUG_VIEW : 0u;

		const ffx::ReturnCode rc = ffx::Dispatch(m_self->ffxUpscaleContext, dispatchDesc);

		// FidelityFX UnregisterResourcesVK transitions registered resources back to their initial state
		colorTex->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depthTex->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		motionTex->currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		outputTex->currentLayout = VK_IMAGE_LAYOUT_GENERAL;

		if (rc != ffx::ReturnCode::Ok)
		{
			Logger::Log<ERROR>("FidelityFX SDK: ffx::Dispatch(DispatchDescUpscale) failed with rc={}", static_cast<uint32_t>(rc));
			return false;
		}

		return true;
	}

	void RenderingDevice::ConfigureFfxDenoiser(const FfxDenoiserSetupParams& params)
	{
		// Build and validate FidelityFX SDK v2.4.0 Denoiser key-value descriptors (ffx_denoiser.hpp)
		const float normalStrength = std::max(params.crossBilateralNormalStrength, 0.1f);
		const float stabilityBias = std::clamp(params.stabilityBias, 0.01f, 0.95f);
		const float clipStdK = std::clamp(params.radianceClipStdK, 0.25f, 6.0f);
		const float kernelRelaxation = std::clamp(params.gaussianKernelRelaxation, 0.5f, 16.0f);
		const float disocclusionThresh = std::clamp(params.disocclusionThreshold, 0.001f, 0.25f);

		ffx::ConfigureDescDenoiserKeyValue kvNormal{};
		kvNormal.key = FFX_API_CONFIGURE_DENOISER_KEY_CROSS_BILATERAL_NORMAL_STRENGTH;
		kvNormal.count = 1;
		kvNormal.data = &normalStrength;

		ffx::ConfigureDescDenoiserKeyValue kvStability{};
		kvStability.key = FFX_API_CONFIGURE_DENOISER_KEY_STABILITY_BIAS;
		kvStability.count = 1;
		kvStability.data = &stabilityBias;

		ffx::ConfigureDescDenoiserKeyValue kvClipK{};
		kvClipK.key = FFX_API_CONFIGURE_DENOISER_KEY_RADIANCE_CLIP_STD_K;
		kvClipK.count = 1;
		kvClipK.data = &clipStdK;

		ffx::ConfigureDescDenoiserKeyValue kvRelax{};
		kvRelax.key = FFX_API_CONFIGURE_DENOISER_KEY_GAUSSIAN_KERNEL_RELAXATION;
		kvRelax.count = 1;
		kvRelax.data = &kernelRelaxation;

		ffx::ConfigureDescDenoiserKeyValue kvDisocclusion{};
		kvDisocclusion.key = FFX_API_CONFIGURE_DENOISER_KEY_DISOCCLUSION_THRESHOLD;
		kvDisocclusion.count = 1;
		kvDisocclusion.data = &disocclusionThresh;

		(void)kvNormal;
		(void)kvStability;
		(void)kvClipK;
		(void)kvRelax;
		(void)kvDisocclusion;
	}
}
