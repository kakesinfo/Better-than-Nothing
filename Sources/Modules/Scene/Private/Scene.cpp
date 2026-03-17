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
	Scene::Scene(const uint64_t size) : m_size(size)
	{
		m_objects.resize(size);
	}

	Scene::~Scene()
	{
		m_objects.clear();
	}

	std::unique_ptr<Scene> Scene::Create(uint64_t size)
	{
		return std::make_unique<Scene>(size);
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

	uint64_t Scene::CreateObject()
	{
		uint64_t id = m_objects.size();
		if (!m_freeIds.empty())
		{
			id = m_freeIds.back();
			m_freeIds.pop_back();
		}

		m_objects[id].id = id;

		return id;
	}

	void Scene::DestroyObject(const uint64_t id)
	{
		if (id >= m_objects.size())
			throw std::out_of_range("No scene object with that id");
		m_freeIds.push_back(id);
	}

	SceneObject& Scene::GetObject(const uint64_t id)
	{
		if (id >= m_objects.size())
			throw std::out_of_range("No scene object with that id");
		return m_objects[id];
	}

	const SceneObject& Scene::GetObject(const uint64_t id) const
	{
		if (id >= m_objects.size())
			throw std::out_of_range("No scene object with that id");
		return m_objects[id];
	}

	std::vector<SceneObject>& Scene::GetObjects()
	{
		return m_objects;
	}

	const std::vector<SceneObject>& Scene::GetObjects() const
	{
		return m_objects;
	}

	uint64_t Scene::GetObjectsCount() const
	{
		return m_size;
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
