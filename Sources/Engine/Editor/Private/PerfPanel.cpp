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

#include "Editor/PerfPanel.hpp"

#include <Common/Metrics.hpp>
#include <Editor/Panel.hpp>
#include <Scene/Scene.hpp>
#include <algorithm>
#include <cstdio>
#include <imgui.h>

namespace BetterThanNothing
{
	PerfPanel::PerfPanel() : Panel("Performance Metrics") {}

	void PerfPanel::OnRender(Scene* scene)
	{
		(void) scene;

		m_updateTimer += ImGui::GetIO().DeltaTime;
		if (m_updateTimer > 0.1f)
		{
			m_cachedMetrics = Metrics::GetLastFrameMetrics();
			m_updateTimer = 0.0f;

			m_fpsHistory[m_historyOffset] = static_cast<float>(m_cachedMetrics.fps);
			m_historyOffset = (m_historyOffset + 1) % kHistorySize;
		}

		const float maxRecorded = *std::ranges::max_element(m_fpsHistory);
		const float scaleMax = std::max(60.0f, maxRecorded * 1.25f);

		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 80.0f);

		char overlay[32];
		std::snprintf(overlay, sizeof(overlay), "FPS: %d", m_cachedMetrics.fps);
		ImGui::PlotLines("##FPS", m_fpsHistory.data(), kHistorySize, m_historyOffset, overlay, 0.0f, scaleMax, ImVec2(-1.0f, 80.0f));

		const float lineY = pos.y + size.y * 0.5f;
		ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x, lineY), ImVec2(pos.x + size.x, lineY), IM_COL32(255, 255, 0, 100));

		ImGui::Text("Frame Time: %.3f ms", m_cachedMetrics.frameTime);
	}
}
