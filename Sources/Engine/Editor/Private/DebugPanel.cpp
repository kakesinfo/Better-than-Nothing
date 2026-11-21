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
	DebugPanel::DebugPanel() : Panel("Render Controls")
	{
		SetPosition(0, 0);
		SetSize(350, 440);
	}

	void DebugPanel::OnRender(Scene* scene)
	{
		m_updateTimer += ImGui::GetIO().DeltaTime;
		if (m_updateTimer > 0.1f)
		{
			m_cachedMetrics = Metrics::GetLastFrameMetrics();
			m_updateTimer = 0.0f;
		}

		auto& settings = RenderSettings::Get();

		// Always-visible compact header: FPS + Debug View quick selector
		ImGui::Text("FPS: %d (%.2f ms) | Draws: %u (Culled: %u)",
					m_cachedMetrics.fps,
					m_cachedMetrics.frameTime,
					m_cachedMetrics.drawCalls,
					m_cachedMetrics.culledDrawCalls);

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
		if (ImGui::Combo("Debug View", &debugViewIdx, kDebugViewNames, IM_ARRAYSIZE(kDebugViewNames)))
			settings.debugView = static_cast<EDebugViewMode>(debugViewIdx);

		ImGui::Spacing();

		if (ImGui::BeginTabBar("RenderControlTabs", ImGuiTabBarFlags_FittingPolicyScroll))
		{
			// --- Tab 1: General & Post-FX ---
			if (ImGui::BeginTabItem("General"))
			{
				ImGui::SeparatorText("Performance");
				static constexpr const char* kFpsLabels[] = {"VSync Only", "30 FPS", "60 FPS", "90 FPS", "120 FPS"};
				static constexpr uint32_t	 kFpsValues[] = {0u, 30u, 60u, 90u, 120u};
				int							 fpsIdx = 2;
				for (int i = 0; i < IM_ARRAYSIZE(kFpsValues); ++i)
				{
					if (settings.targetFps == kFpsValues[i])
					{
						fpsIdx = i;
						break;
					}
				}
				if (ImGui::Combo("FPS Limit", &fpsIdx, kFpsLabels, IM_ARRAYSIZE(kFpsLabels)))
					settings.targetFps = kFpsValues[fpsIdx];

				ImGui::TextDisabled("Vertices: %u | Total draws: %llu",
									m_cachedMetrics.verticesRendered,
									static_cast<unsigned long long>(m_cachedMetrics.totalDrawCalls));
				ImGui::TextDisabled("Frame count: %u | VSync: FIFO", m_cachedMetrics.frameCount);

				ImGui::SeparatorText("Post-Processing");
				static constexpr const char* kToneMapNames[] = {"AgX", "ACES", "Reinhard"};
				int							 toneMapIdx = static_cast<int>(settings.toneMapMode);
				if (ImGui::Combo("Tone Map", &toneMapIdx, kToneMapNames, IM_ARRAYSIZE(kToneMapNames)))
					settings.toneMapMode = static_cast<EToneMapMode>(toneMapIdx);

				ImGui::Checkbox("TAA", &settings.enableTAA);
				if (settings.enableTAA)
					ImGui::SliderFloat("TAA Blend", &settings.taaBlendFactor, 0.02f, 0.5f, "%.2f");

				ImGui::EndTabItem();
			}

			// --- Tab 2: Physical Camera ---
			if (ImGui::BeginTabItem("Camera"))
			{
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

					ImGui::SeparatorText("Presets");
					if (ImGui::SmallButton("Sunny 16"))
						cam->SetPhysicalExposure(16.0f, 1.0f / 125.0f, 100.0f, 0.0f);
					ImGui::SameLine();
					if (ImGui::SmallButton("Courtyard"))
						cam->SetPhysicalExposure(11.0f, 1.0f / 125.0f, 100.0f, 0.0f);
					ImGui::SameLine();
					if (ImGui::SmallButton("Indoor"))
						cam->SetPhysicalExposure(2.8f, 1.0f / 60.0f, 400.0f, 0.0f);
				}

				ImGui::EndTabItem();
			}

			// --- Tab 3: Sky & Atmosphere ---
			if (ImGui::BeginTabItem("Sky"))
			{
				ImGui::SeparatorText("Sun");
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
						sunChanged |= ImGui::SliderFloat("Elevation", &elevationDeg, -5.0f, 89.0f, "%.1f deg");
						sunChanged |= ImGui::SliderFloat("Azimuth", &azimuthDeg, 0.0f, 360.0f, "%.1f deg");
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

				ImGui::SeparatorText("Hillaire Atmosphere & IBL");
				ImGui::SliderFloat("Irradiance Scale", &settings.irradianceIntensity, 0.0f, 3.0f, "%.2f");
				ImGui::SliderFloat("Altitude", &settings.atmosphereAltitudeKm, 0.01f, 80.0f, "%.2f km", ImGuiSliderFlags_Logarithmic);
				ImGui::SliderFloat("Mie Anisotropy", &settings.atmosphereMieAnisotropy, 0.0f, 0.95f, "%.2f");
				ImGui::SliderFloat("Multi-Scatter", &settings.atmosphereMultiScatteringFactor, 0.0f, 2.0f, "%.2f");
				ImGui::SliderFloat("Atmo Density", &settings.atmosphereDensityScale, 0.1f, 3.0f, "%.2f");

				ImGui::EndTabItem();
			}

			// --- Tab 4: Shadows & AO ---
			if (ImGui::BeginTabItem("Shadows"))
			{
				ImGui::SeparatorText("RT Shadows (AMD FFX Denoiser)");
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

				ImGui::SeparatorText("Bend Studio SSS");
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

				ImGui::EndTabItem();
			}

			// --- Tab 5: Water & Volumetric Fog ---
			if (ImGui::BeginTabItem("Water & Fog"))
			{
				ImGui::SeparatorText("Water Surface");
				ImGui::SliderFloat("Wave Scale", &settings.waterWaveScale, 0.0f, 2.5f, "%.2f");
				ImGui::SliderFloat("Foam", &settings.waterFoamIntensity, 0.0f, 2.0f, "%.2f");

				ImGui::SeparatorText("Volumetric Fog (WsfBDf)");
				ImGui::Checkbox("Volumetric Fog", &settings.enableVolumetricFog);
				if (settings.enableVolumetricFog)
				{
					ImGui::Checkbox("Restrict to Sponza", &settings.fogRestrictToSponza);
					ImGui::Checkbox("Blue Noise Jitter", &settings.fogUseBlueNoise);
					if (settings.rtSupported)
						ImGui::Checkbox("RT God Rays", &settings.fogUseRTShadows);
					ImGui::SliderFloat("Density", &settings.fogDensity, 0.001f, 0.08f, "%.4f");
					ImGui::SliderFloat("Height Falloff", &settings.fogHeightFalloff, 0.01f, 0.5f, "%.3f");
					ImGui::SliderFloat("Base Height", &settings.fogBaseHeight, -5.0f, 12.0f, "%.1f m");
					ImGui::SliderFloat("Max Distance", &settings.fogMaxDistance, 20.0f, 200.0f, "%.0f m");
					ImGui::SliderFloat("Anisotropy (g)", &settings.fogAnisotropy, 0.0f, 0.90f, "%.2f");
					ImGui::SliderFloat("Mist Noise", &settings.fogNoiseStrength, 0.0f, 1.0f, "%.2f");
					int steps = static_cast<int>(settings.fogStepCount);
					if (ImGui::SliderInt("Raymarch Steps", &steps, 8, 48))
						settings.fogStepCount = static_cast<uint32_t>(steps);
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}
