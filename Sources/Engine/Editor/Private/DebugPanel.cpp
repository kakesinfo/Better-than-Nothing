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

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING

#include <Common/Metrics.hpp>
#include <Common/RenderSettings.hpp>
#include <Editor/DebugPanel.hpp>
#include <Editor/Panel.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <algorithm>
#include <cmath>
#include <imgui.h>

namespace BetterThanNothing
{
	DebugPanel::DebugPanel() : Panel("Debug Panel")
	{
		SetPosition(0, 0);
		SetSize(290, 660);
	}

	void DebugPanel::OnRender(Scene* scene)
	{
		m_updateTimer += ImGui::GetIO().DeltaTime;
		if (m_updateTimer > 0.1f)
		{
			m_cachedMetrics = Metrics::GetLastFrameMetrics();
			m_updateTimer = 0.0f;
		}

		ImGui::SeparatorText("Rendering");
		ImGui::Text("Vertices: %u", m_cachedMetrics.verticesRendered);
		ImGui::Text("Draw calls: %u", m_cachedMetrics.drawCalls);
		ImGui::Text("Culled: %u", m_cachedMetrics.culledDrawCalls);
		ImGui::Text("Total draw calls: %llu", static_cast<unsigned long long>(m_cachedMetrics.totalDrawCalls));

		ImGui::SeparatorText("Frames");
		ImGui::Text("Frame count: %u", m_cachedMetrics.frameCount);
		ImGui::Text("Frame time: %.2f ms", m_cachedMetrics.frameTime);
		ImGui::Text("FPS: %d", m_cachedMetrics.fps);

		auto& settings = RenderSettings::Get();

		ImGui::SeparatorText("Physical Camera");
		ImGui::Text("Pos: x=%.1f y=%.1f z=%.1f", m_cachedMetrics.cameraX, m_cachedMetrics.cameraY, m_cachedMetrics.cameraZ);
		if (scene && scene->HasCamera())
		{
			Camera* cam = scene->GetCamera();
			float	aperture = cam->GetAperture();
			float	shutterDenom = 1.0f / cam->GetShutterSpeed();
			float	iso = cam->GetIso();
			float	evComp = cam->GetEvCompensation();

			if (ImGui::SliderFloat("Aperture", &aperture, 1.4f, 22.0f, "f/%.1f"))
				cam->SetAperture(aperture);
			if (ImGui::SliderFloat("Shutter", &shutterDenom, 1.0f, 4000.0f, "1/%.0f s", ImGuiSliderFlags_Logarithmic))
				cam->SetShutterSpeed(1.0f / shutterDenom);
			if (ImGui::SliderFloat("ISO", &iso, 50.0f, 6400.0f, "%.0f", ImGuiSliderFlags_Logarithmic))
				cam->SetIso(iso);
			if (ImGui::SliderFloat("EV Comp", &evComp, -5.0f, 5.0f, "%+.2f EV"))
				cam->SetEvCompensation(evComp);

			ImGui::TextDisabled("EV100: %.2f  (Pre-Exp: %.2e)", cam->GetEV100(), cam->GetExposure());

			if (ImGui::SmallButton("Sunny 16"))
				cam->SetPhysicalExposure(16.0f, 1.0f / 125.0f, 100.0f, 0.0f);
			ImGui::SameLine();
			if (ImGui::SmallButton("Courtyard"))
				cam->SetPhysicalExposure(11.0f, 1.0f / 125.0f, 100.0f, 0.0f);
			ImGui::SameLine();
			if (ImGui::SmallButton("Indoor"))
				cam->SetPhysicalExposure(2.8f, 1.0f / 60.0f, 400.0f, 0.0f);
		}

		ImGui::SeparatorText("Atmosphere & Irradiance");
		if (scene)
		{
			if (auto dirView = scene->GetView<DirectionalLightComponent>(); dirView.begin() != dirView.end())
			{
				auto&			dirLight = dirView.get<DirectionalLightComponent>(*dirView.begin());
				const Vector3	sunDir = (dirLight.direction * -1.0f).Normalized();
				constexpr float kRadToDeg = 180.0f / 3.14159265359f;
				constexpr float kDegToRad = 3.14159265359f / 180.0f;

				float elevationDeg = std::asin(std::clamp(sunDir.GetY(), -1.0f, 1.0f)) * kRadToDeg;
				float azimuthDeg = std::atan2(sunDir.GetZ(), sunDir.GetX()) * kRadToDeg;
				if (azimuthDeg < 0.0f)
					azimuthDeg += 360.0f;

				bool sunChanged = false;
				sunChanged |= ImGui::SliderFloat("Sun Elevation", &elevationDeg, -5.0f, 89.0f, "%.1f deg");
				sunChanged |= ImGui::SliderFloat("Sun Azimuth", &azimuthDeg, 0.0f, 360.0f, "%.1f deg");
				if (sunChanged)
				{
					const float	  elevRad = elevationDeg * kDegToRad;
					const float	  azimRad = azimuthDeg * kDegToRad;
					const Vector3 newSunDir(
						std::cos(elevRad) * std::cos(azimRad),
						std::sin(elevRad),
						std::cos(elevRad) * std::sin(azimRad)
					);
					dirLight.direction = (newSunDir * -1.0f).Normalized();
				}
			}
		}
		ImGui::SliderFloat("Irradiance Scale", &settings.irradianceIntensity, 0.0f, 3.0f, "%.2f");
		ImGui::SliderFloat("Altitude", &settings.atmosphereAltitudeKm, 0.01f, 80.0f, "%.2f km", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Mie Anisotropy", &settings.atmosphereMieAnisotropy, 0.0f, 0.95f, "%.2f");
		ImGui::SliderFloat("Multi-Scatter", &settings.atmosphereMultiScatteringFactor, 0.0f, 2.0f, "%.2f");
		ImGui::SliderFloat("Atmo Density", &settings.atmosphereDensityScale, 0.1f, 3.0f, "%.2f");

		ImGui::SeparatorText("Ambient Occlusion");
		static constexpr const char* kAoModeNames[] = {"Off", "GTAO (Screen-Space)", "RTAO (Ray Query)"};
		int							 aoModeIdx = static_cast<int>(settings.aoMode);
		if (ImGui::Combo("AO Mode", &aoModeIdx, kAoModeNames, settings.rtSupported ? 3 : 2))
			settings.aoMode = static_cast<EAOMode>(aoModeIdx);
		if (settings.aoMode != EAOMode::Off)
		{
			ImGui::SliderFloat("AO Radius", &settings.aoRadius, 0.1f, 5.0f, "%.2f m");
			ImGui::SliderFloat("AO Power", &settings.aoPower, 0.25f, 4.0f, "%.2f");
		}

		ImGui::SeparatorText("Deferred + FFX RT Pipeline");

		static constexpr const char* kDebugViewNames[] = {
			"Final",
			"Albedo",
			"Octahedral Normal",
			"Metallic",
			"Roughness",
			"Motion Vectors",
			"RT Shadows (FFX)",
			"Ambient Occlusion",
			"Diffuse Irradiance",
			"Depth",
			"Screen Space Shadows (Bend)",
			"Volumetric Fog (WsfBDf)",
		};
		int debugViewIdx = static_cast<int>(settings.debugView);
		if (ImGui::Combo("View", &debugViewIdx, kDebugViewNames, IM_ARRAYSIZE(kDebugViewNames)))
			settings.debugView = static_cast<EDebugViewMode>(debugViewIdx);

		static constexpr const char* kToneMapNames[] = {"AgX", "ACES", "Reinhard"};
		int							 toneMapIdx = static_cast<int>(settings.toneMapMode);
		if (ImGui::Combo("Tone Map", &toneMapIdx, kToneMapNames, IM_ARRAYSIZE(kToneMapNames)))
			settings.toneMapMode = static_cast<EToneMapMode>(toneMapIdx);

		ImGui::Checkbox("TAA", &settings.enableTAA);
		if (settings.enableTAA)
			ImGui::SliderFloat("TAA Blend", &settings.taaBlendFactor, 0.02f, 0.5f, "%.2f");

		if (settings.rtSupported)
		{
			ImGui::Checkbox("RT Shadows", &settings.enableRTShadows);
			if (settings.enableRTShadows)
			{
				ImGui::Checkbox("FFX Shadow Denoiser", &settings.enableShadowDenoiser);
				ImGui::SliderFloat("Sun Radius", &settings.sunAngularRadius, 0.0f, 0.06f, "%.3f");
			}
		}
		else
		{
			ImGui::TextDisabled("HW Ray Query: unsupported");
		}

		ImGui::SeparatorText("Bend Studio Screen Space Shadows");
		ImGui::Checkbox("Bend SSS", &settings.enableBendSSS);
		if (settings.enableBendSSS)
		{
			ImGui::SliderFloat("Thickness", &settings.sssSurfaceThickness, 0.0005f, 0.05f, "%.4f");
			ImGui::SliderFloat("Bilinear Thresh", &settings.sssBilinearThreshold, 0.001f, 0.1f, "%.3f");
			ImGui::SliderFloat("Contrast", &settings.sssShadowContrast, 1.0f, 8.0f, "%.2f");
			ImGui::Checkbox("Ignore Edge Pixels", &settings.sssIgnoreEdgePixels);
			ImGui::Checkbox("Bilinear Offset Mode", &settings.sssBilinearOffsetMode);
			ImGui::Checkbox("Early Out", &settings.sssUseEarlyOut);
			static constexpr const char* kSssDebugNames[] = {"None", "Edge Mask", "Thread Index", "Wave Index"};
			int							 sssDebugIdx = static_cast<int>(settings.sssDebugMode);
			if (ImGui::Combo("SSS Debug", &sssDebugIdx, kSssDebugNames, IM_ARRAYSIZE(kSssDebugNames)))
				settings.sssDebugMode = static_cast<uint32_t>(sssDebugIdx);
		}

		ImGui::SeparatorText("Water");
		ImGui::SliderFloat("Wave Scale", &settings.waterWaveScale, 0.0f, 2.5f, "%.2f");
		ImGui::SliderFloat("Foam", &settings.waterFoamIntensity, 0.0f, 2.0f, "%.2f");

		ImGui::SeparatorText("Volumetric Fog (WsfBDf Blue Noise)");
		ImGui::Checkbox("Volumetric Fog", &settings.enableVolumetricFog);
		if (settings.enableVolumetricFog)
		{
			ImGui::Checkbox("Blue Noise Jitter", &settings.fogUseBlueNoise);
			if (settings.rtSupported)
				ImGui::Checkbox("RT Volumetric God Rays", &settings.fogUseRTShadows);
			ImGui::SliderFloat("Fog Density", &settings.fogDensity, 0.001f, 0.05f, "%.4f");
			ImGui::SliderFloat("Height Falloff", &settings.fogHeightFalloff, 0.01f, 0.5f, "%.3f");
			ImGui::SliderFloat("Base Height", &settings.fogBaseHeight, -10.0f, 15.0f, "%.1f m");
			ImGui::SliderFloat("Max Distance", &settings.fogMaxDistance, 30.0f, 350.0f, "%.0f m");
			ImGui::SliderFloat("Anisotropy (g)", &settings.fogAnisotropy, 0.0f, 0.90f, "%.2f");
			ImGui::SliderFloat("Mist Noise", &settings.fogNoiseStrength, 0.0f, 1.0f, "%.2f");
			int steps = static_cast<int>(settings.fogStepCount);
			if (ImGui::SliderInt("Raymarch Steps", &steps, 8, 64))
				settings.fogStepCount = static_cast<uint32_t>(steps);
		}
	}
}
