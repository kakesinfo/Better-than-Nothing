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
	class Camera
	{
	public:
		Camera(float fov, float nearPlane, float farPlane);
		~Camera() = default;

		void SetPosition(const glm::vec3& position);
		void SetOrientation(const glm::quat& orientation);

		void Move(const glm::vec3& delta);
		void Rotate(float yaw, float pitch);

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix(float aspectRatio) const;

		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;
		glm::vec3 GetUp() const;

		glm::vec3 GetPosition() const;

	private:
		float		m_fov;
		float		m_nearPlane;
		float		m_farPlane;

		glm::vec3	m_position{};
		glm::quat	m_orientation{};
	};
}