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

#pragma once

#include "Math/Matrix4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector3.hpp"

namespace BetterThanNothing
{
	class Camera
	{
	public:
		Camera(float fov, float nearPlane, float farPlane);
		~Camera() = default;

		void SetPosition(const Vector3& position);
		void SetOrientation(const Quaternion& orientation);
		void SetFov(float fov);
		void SetExposure(float exposure);
		void SetPhysicalExposure(float aperture, float shutterSpeed, float iso, float evCompensation = 0.0f);

		void Move(const Vector3& delta);
		void Rotate(float yaw, float pitch);

		[[nodiscard]] const Matrix4& GetViewMatrix() const;
		[[nodiscard]] Matrix4		 GetProjectionMatrix(float aspectRatio) const;

		[[nodiscard]] Vector3	 GetForward() const;
		[[nodiscard]] Vector3	 GetRight() const;
		[[nodiscard]] Vector3	 GetUp() const;
		[[nodiscard]] Vector3	 GetPosition() const;
		[[nodiscard]] Quaternion GetOrientation() const;
		[[nodiscard]] float		 GetFov() const;
		[[nodiscard]] float		 GetNearPlane() const;
		[[nodiscard]] float		 GetFarPlane() const;
		[[nodiscard]] float		 GetExposure() const;
		[[nodiscard]] float		 GetEV100() const;
		[[nodiscard]] float		 GetAperture() const { return m_aperture; }
		[[nodiscard]] float		 GetShutterSpeed() const { return m_shutterSpeed; }
		[[nodiscard]] float		 GetISO() const { return m_iso; }
		[[nodiscard]] float		 GetEVCompensation() const { return m_evCompensation; }

	private:
		void UpdateOrientationFromAngles();

		float m_fov;
		float m_nearPlane;
		float m_farPlane;
		float m_aperture = 11.0f;
		float m_shutterSpeed = 1.0f / 125.0f;
		float m_iso = 100.0f;
		float m_evCompensation = 0.0f;

		Vector3	   m_position;
		Quaternion m_orientation;
		float	   m_yaw = 0.0f;
		float	   m_pitch = 0.0f;

		mutable Matrix4 m_viewMatrix = Matrix4::Identity;
		mutable bool	m_viewDirty = true;
	};
}
