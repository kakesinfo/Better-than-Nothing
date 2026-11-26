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
#include <cmath>
#include <imgui.h>

namespace BetterThanNothing
{
	DebugPanel::DebugPanel() : Panel("Debug Panel")
	{
		SetPosition(0, 0);
		SetSize(340, 420);
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

		// Permanent header: FPS, draw calls & Debug View selector
		ImGui::Text("FPS: %d (%.2f ms) | Draws: %u (Culled: %u)",
					m_cachedMetrics.fps,
					m_cachedMetrics.frameTime,
					m_cachedMetrics.drawCalls,
					m_cachedMetrics.culledDrawCalls);

		static constexpr const char* kDebugViewNames[] = {
			"Final",
			"Albedo",
			"World Normal",
			"Roughness",
			"Metallic",
			"Depth",
			"Motion Vectors",
			"Ambient Occlusion",
			"RT Shadows",
			"Diffuse Irradiance",
			"Screen-Space Shadows",
			"Cloud Transmittance",
		};
		int currentDebugView = static_cast<int>(settings.debugView);
		if (ImGui::Combo("Debug View", &currentDebugView, kDebugViewNames, IM_ARRAYSIZE(kDebugViewNames)))
			settings.debugView = static_cast<EDebugViewMode>(currentDebugView);

		ImGui::Separator();

		if (ImGui::BeginTabBar("DebugTabs"))
		{
			if (ImGui::BeginTabItem("General"))
			{
				static constexpr const char* kToneMapNames[] = {"AgX", "ACES", "Reinhard"};
				int							 toneMap = static_cast<int>(settings.toneMapMode);
				if (ImGui::Combo("Tone Map", &toneMap, kToneMapNames, IM_ARRAYSIZE(kToneMapNames)))
					settings.toneMapMode = static_cast<EToneMapMode>(toneMap);

				ImGui::Checkbox("Enable TAA", &settings.enableTAA);
				ImGui::SliderInt("Target FPS (0 = Unlimited)", &settings.targetFps, 0, 240);

				ImGui::SeparatorText("Metrics");
				ImGui::Text("Vertices: %u", m_cachedMetrics.verticesRendered);
				ImGui::Text("Total draw calls: %llu", static_cast<unsigned long long>(m_cachedMetrics.totalDrawCalls));
				ImGui::Text("Frame count: %u", m_cachedMetrics.frameCount);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::Text("Pos: x=%.1f y=%.1f z=%.1f", m_cachedMetrics.cameraX, m_cachedMetrics.cameraY, m_cachedMetrics.cameraZ);
				if (scene && scene->HasCamera())
				{
					Camera* cam = scene->GetCamera();
					float	aperture = cam->GetAperture();
					float	shutterInv = 1.0f / std::max(cam->GetShutterSpeed(), 1e-5f);
					float	iso = cam->GetISO();
					float	evComp = cam->GetEVCompensation();

					bool changed = false;
					changed |= ImGui::SliderFloat("Aperture (f/)", &aperture, 1.4f, 22.0f, "f/%.1f");
					changed |= ImGui::SliderFloat("Shutter (1/s)", &shutterInv, 15.0f, 4000.0f, "1/%.0f s");
					changed |= ImGui::SliderFloat("ISO", &iso, 50.0f, 3200.0f, "%.0f");
					changed |= ImGui::SliderFloat("EV Comp", &evComp, -4.0f, 4.0f, "%.2f");
					if (changed)
						cam->SetPhysicalExposure(aperture, 1.0f / std::max(shutterInv, 1.0f), iso, evComp);

					ImGui::Text("EV100: %.2f | Exposure: %.5f", cam->GetEV100(), cam->GetExposure());
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sky"))
			{
				ImGui::SliderFloat("Sky Luminance (nits)", &settings.skyLuminanceNits, 1000.0f, 25000.0f, "%.0f");
				ImGui::Checkbox("Enable Clouds", &settings.enableClouds);
				ImGui::SliderFloat("Cloud Coverage", &settings.cloudCoverage, 0.0f, 1.0f, "%.2f");
				ImGui::SliderFloat("Cloud Speed", &settings.cloudSpeed, 0.0f, 30.0f, "%.1f m/s");

				if (scene)
				{
					auto lightView = scene->GetView<DirectionalLightComponent>();
					for (auto [entity, dirLight] : lightView.each())
					{
						float dir[3] = {dirLight.direction.GetX(), dirLight.direction.GetY(), dirLight.direction.GetZ()};
						if (ImGui::SliderFloat3("Sun Direction", dir, -1.0f, 1.0f))
							dirLight.direction = Vector3(dir[0], dir[1], dir[2]).Normalized();
						ImGui::SliderFloat("Sun Illuminance (lux)", &dirLight.illuminanceLux, 1000.0f, 150000.0f, "%.0f");
						break;
					}
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Shadows"))
			{
				ImGui::Checkbox("Enable RT Shadows", &settings.enableRTShadows);
				ImGui::SliderFloat("Sun Angular Radius", &settings.sunAngularRadius, 0.001f, 0.05f, "%.4f rad");
				ImGui::Checkbox("Enable Screen-Space Shadows", &settings.enableScreenSpaceShadows);

				static constexpr const char* kSssDebugNames[] = {"Off", "Edge Mask", "Thread Index", "Wave Index"};
				ImGui::Combo("SSS Debug Mode", &settings.sssDebugMode, kSssDebugNames, IM_ARRAYSIZE(kSssDebugNames));

				ImGui::SeparatorText("Ambient Occlusion");
				static constexpr const char* kAoModes[] = {"Off", "RTAO", "GTAO"};
				int							 aoMode = static_cast<int>(settings.aoMode);
				if (ImGui::Combo("AO Mode", &aoMode, kAoModes, IM_ARRAYSIZE(kAoModes)))
					settings.aoMode = static_cast<EAOMode>(aoMode);
				ImGui::SliderFloat("AO Radius", &settings.aoRadius, 0.25f, 5.0f, "%.2f m");
				ImGui::SliderFloat("AO Strength", &settings.aoStrength, 0.0f, 2.5f, "%.2f");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Nature & Fog"))
			{
				ImGui::SliderFloat("Grass Fade Distance", &settings.grassFadeDistance, 30.0f, 250.0f, "%.0f m");
				ImGui::Checkbox("Enable Fog", &settings.enableFog);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}
