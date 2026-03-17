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

#include <memory>
#include <vector>
#include <optional>

#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Module_Data/Graphics/PointLightData.hpp"

namespace BetterThanNothing
{
	class Camera;
	struct SceneObject;

	class Scene
	{
	public:
		explicit Scene(uint64_t size);
		~Scene();

		static std::unique_ptr<Scene> Create(uint64_t size);

		void InitCamera(float fov, float nearPlane, float farPlane);
		[[nodiscard]] Camera* GetCamera() const;

		uint64_t CreateObject();
		void DestroyObject(uint64_t id);

		[[nodiscard]] SceneObject& GetObject(uint64_t id);
		[[nodiscard]] const SceneObject& GetObject(uint64_t id) const;
		[[nodiscard]] std::vector<SceneObject>& GetObjects();
		[[nodiscard]] const std::vector<SceneObject>& GetObjects() const;
		[[nodiscard]] uint64_t GetObjectsCount() const;

		void SetDirectionalLight(const DirectionalLightData& data);
		void AddPointLight(const PointLightData& data);

		[[nodiscard]] const DirectionalLightData& GetDirectionalLight() const;
		[[nodiscard]] uint32_t GetPointLightCount() const;
		[[nodiscard]] const std::vector<PointLightData>& GetPointLights() const;
		[[nodiscard]] const PointLightData& GetPointLight(uint32_t index) const;

	private:
		uint64_t m_size;
		std::unique_ptr<Camera> m_camera;
		std::vector<SceneObject> m_objects;
		std::vector<uint64_t> m_freeIds;

		std::optional<DirectionalLightData> m_directionalLight;
		std::vector<PointLightData> m_pointLights;
	};
}
