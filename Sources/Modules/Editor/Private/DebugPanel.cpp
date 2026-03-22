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

#include <Editor/DebugPanel.hpp>
#include <Editor/Panel.hpp>
#include <Module_Base/Metrics.hpp>
#include <imgui.h>

namespace BetterThanNothing
{
	DebugPanel::DebugPanel(const std::string_view name) : Panel(name) {}

	void DebugPanel::OnRender()
	{
		static auto updateTimed = 0.0f;
		static auto cachedMetrics = Metrics::GetLastFrameMetrics();

		updateTimed += ImGui::GetIO().DeltaTime;
		if (updateTimed > 0.1f)
		{
			cachedMetrics = Metrics::GetLastFrameMetrics();
			updateTimed = 0.0f;
		}

		ImGui::SeparatorText("Rendering");
		ImGui::Text("Vertices: %d", cachedMetrics.verticesRendered);
		ImGui::Text("Draw calls: %d", cachedMetrics.drawCalls);
		ImGui::Text("Total draw calls: %zu", cachedMetrics.totalDrawCalls);
		ImGui::SeparatorText("Frames");
		ImGui::Text("Frame count: %d", cachedMetrics.frameCount);
		ImGui::Text("Frame time: %.2f ms", cachedMetrics.frameTime);
		ImGui::Text("FPS: %d", cachedMetrics.fps);
		ImGui::SeparatorText("Camera");
		ImGui::Text("Pos: x=%.1f y=%.1f z=%.1f", cachedMetrics.cameraX, cachedMetrics.cameraY, cachedMetrics.cameraZ);
	}
}
