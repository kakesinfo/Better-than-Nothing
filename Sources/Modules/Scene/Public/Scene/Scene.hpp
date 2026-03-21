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

#include <entt/entt.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "Module_Data/Graphics/DirectionalLightData.hpp"
#include "Module_Data/Graphics/PointLightData.hpp"

namespace BetterThanNothing
{
	class ResourceManager;
	class Camera;
	class SceneObject;

	class Scene
	{
	public:
		explicit Scene(ResourceManager* resourceManager);
		~Scene() = default;

		static std::unique_ptr<Scene> Create(ResourceManager* resourceManager);

		void				  InitCamera(float fov, float nearPlane, float farPlane);
		[[nodiscard]] Camera* GetCamera() const;

		SceneObject CreateObject();

		template<typename... Components>
		[[nodiscard]] auto GetView()
		{
			return m_registry.view<Components...>();
		}

		template<typename... Components>
		[[nodiscard]] auto GetView() const
		{
			return m_registry.view<Components...>();
		}

		void SetDirectionalLight(const DirectionalLightData& data);
		void AddPointLight(const PointLightData& data);

		[[nodiscard]] const DirectionalLightData&		 GetDirectionalLight() const;
		[[nodiscard]] uint32_t							 GetPointLightCount() const;
		[[nodiscard]] const std::vector<PointLightData>& GetPointLights() const;
		[[nodiscard]] const PointLightData&				 GetPointLight(uint32_t index) const;

	private:
		ResourceManager*					m_resourceManager;
		entt::registry						m_registry;
		std::unique_ptr<Camera>				m_camera;
		std::optional<DirectionalLightData> m_directionalLight;
		std::vector<PointLightData>			m_pointLights;
	};
}
