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
#include <imgui.h>

namespace BetterThanNothing
{
	PerfPanel::PerfPanel() : Panel("Performance Metrics") {}

	void PerfPanel::OnRender(Scene* scene)
	{
		constexpr int value_count = 100;
		static float  values[value_count] = {0};
		static int	  values_offset = 0;

		static auto updateTimed = 0.0f;
		static auto cachedMetrics = Metrics::GetLastFrameMetrics();

		updateTimed += ImGui::GetIO().DeltaTime;
		if (updateTimed > 0.1f)
		{
			cachedMetrics = Metrics::GetLastFrameMetrics();
			updateTimed = 0.0f;

			values[values_offset] = static_cast<float>(cachedMetrics.fps);
			values_offset = (values_offset + 1) % value_count;
		}

		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 80);

		char overlay[32];
		sprintf(overlay, "FPS: %d", cachedMetrics.fps);
		ImGui::PlotLines("##FPS", values, value_count, values_offset, overlay, 120, 360, ImVec2(-1, 80));

		const float lineY = pos.y + size.y * 0.5f;
		ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x, lineY), ImVec2(pos.x + size.x, lineY), IM_COL32(255, 255, 0, 100));

		ImGui::Text("Frame Time: %.3f ms", cachedMetrics.frameTime);
	}
}
