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
#include <Common/Metrics.hpp>
#include <Editor/Panel.hpp>
#include <imgui.h>

namespace BetterThanNothing
{
	DebugPanel::DebugPanel() : Panel("Debug Panel")
	{
		SetPosition(0, 0);
		SetSize(200, 250);
	}

	void DebugPanel::OnRender(Scene* scene)
	{
		(void) scene;

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
		ImGui::SeparatorText("Camera");
		ImGui::Text("Pos: x=%.1f y=%.1f z=%.1f", m_cachedMetrics.cameraX, m_cachedMetrics.cameraY, m_cachedMetrics.cameraZ);
	}
}
