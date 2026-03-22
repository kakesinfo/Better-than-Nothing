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

#include "Scene/Scene.hpp"
#include "Scene/Camera.hpp"
#include "Scene/SceneObject.hpp"

namespace BetterThanNothing
{
	Scene::Scene(ResourceManager* resourceManager): m_resourceManager(resourceManager)
	{

	}

	std::unique_ptr<Scene> Scene::Create(ResourceManager* resourceManager)
	{
		return std::make_unique<Scene>(resourceManager);
	}

	void Scene::InitCamera(const float fov, const float nearPlane, const float farPlane)
	{
		m_camera = std::make_unique<Camera>(fov, nearPlane, farPlane);
	}

	Camera* Scene::GetCamera() const
	{
		if (!m_camera)
			throw std::runtime_error("No camera");
		return m_camera.get();
	}

	SceneObject Scene::CreateObject()
	{
		return GetObject(m_registry.create());
	}

	SceneObject Scene::GetObject(const entt::entity entity)
	{
		return SceneObject(m_resourceManager, &m_registry, entity);
	}

	void Scene::SetDirectionalLight(const DirectionalLightData& data)
	{
		m_directionalLight.emplace(data);
	}

	void Scene::AddPointLight(const PointLightData& data)
	{
		m_pointLights.push_back(data);
	}

	const DirectionalLightData& Scene::GetDirectionalLight() const
	{
		if (!m_directionalLight.has_value())
			throw std::runtime_error("No directional light");
		return m_directionalLight.value();
	}

	uint32_t Scene::GetPointLightCount() const
	{
		return m_pointLights.size();
	}

	const std::vector<PointLightData>& Scene::GetPointLights() const
	{
		return m_pointLights;
	}

	const PointLightData& Scene::GetPointLight(const uint32_t index) const
	{
		if (index > m_pointLights.size())
			throw std::out_of_range("No point light with that index");
		return m_pointLights.at(index);
	}
}
