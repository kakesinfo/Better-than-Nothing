/*
*	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#pragma once

#include <cmath>

namespace BetterThanNothing
{
	struct RenderMetricsData
	{
		std::uint32_t	frameCount = 0;
		std::uint32_t	drawCalls = 0;
		std::uint32_t	verticesRendered = 0;
		std::uint64_t	totalDrawCalls = 0;
		std::uint64_t	totalVerticesRendered = 0;
		float			frameTime = 0;
		int				fps = 0;

		float			cameraX;
		float			cameraY;
		float			cameraZ;
	};

	class RenderMetrics
	{
	public:
		static void AddDrawCall(std::uint32_t vertexCount)
		{
			s_currentFrameMetrics.drawCalls++;
			s_currentFrameMetrics.verticesRendered += vertexCount;
		}

		static void SetFrameTime(const float frameTime)
		{
			s_currentFrameMetrics.frameTime = frameTime;
		}

		static void SetFps(const float fps)
		{
			s_currentFrameMetrics.fps = static_cast<int>(std::round(fps));
		}

		static void SetCameraCoordinates(const float x, const float y, const float z)
		{
			s_currentFrameMetrics.cameraX = x;
			s_currentFrameMetrics.cameraY = y;
			s_currentFrameMetrics.cameraZ = z;
		}

		static void BeginRecording()
		{
			std::lock_guard lock(s_mutex);

			s_currentFrameMetrics.drawCalls = 0;
			s_currentFrameMetrics.verticesRendered = 0;
			s_currentFrameMetrics.frameTime = 0;
		}

		static void EndRecording()
		{
			std::lock_guard lock(s_mutex);

			s_currentFrameMetrics.frameCount++;
			s_currentFrameMetrics.totalDrawCalls += s_currentFrameMetrics.drawCalls;
			s_currentFrameMetrics.totalVerticesRendered += s_currentFrameMetrics.verticesRendered;

			s_lastFrameMetrics = s_currentFrameMetrics;
		}

		static RenderMetricsData GetLastFrameMetrics()
		{
			std::lock_guard lock(s_mutex);
			return s_lastFrameMetrics;
		}

	private:
		static inline RenderMetricsData s_currentFrameMetrics = {};
		static inline RenderMetricsData s_lastFrameMetrics = {};
		static inline std::mutex s_mutex = {};
	};
}
