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

namespace BetterThanNothing
{
	class Camera;
	enum class EKey : std::int16_t;
	enum class EMouseButton : std::int16_t;

	class UpdateContext
	{
	public:
		explicit UpdateContext(Camera* camera, const float deltaTime) : m_camera(camera), m_deltaTime(deltaTime) {}
		~UpdateContext() = default;

		void RotateCamera(float yaw, float pitch) const;
		void MoveCamera(const glm::vec3& delta) const;

		[[nodiscard]] glm::vec3 GetCameraForward() const;
		[[nodiscard]] glm::vec3 GetCameraRight() const;

		[[nodiscard]] bool		IsKeyPressed(EKey key) const;
		[[nodiscard]] bool		IsMouseButtonPressed(EMouseButton button) const;
		[[nodiscard]] glm::vec2 GetMouseDelta() const;

		[[nodiscard]] float GetDeltaTime() const;

	private:
		Camera* m_camera;
		float	m_deltaTime;
	};
}
