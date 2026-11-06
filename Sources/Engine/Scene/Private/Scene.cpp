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
#include <ResourceManager/ResourceManager.hpp>
#include <filesystem>

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
		return SceneObject(&m_registry, entity);
	}

	void Scene::DestroyObject(SceneObject& object)
	{
		object.Destroy();
	}

	void Scene::UpdateTransforms()
	{
		auto updateEntityHierarchy = [&](auto&& self, entt::entity entity, const Matrix4& parentWorld) -> void {
			Matrix4 currentWorld = parentWorld;
			if (m_registry.all_of<TransformComponent>(entity))
			{
				auto& transform = m_registry.get<TransformComponent>(entity);
				currentWorld = parentWorld * transform.localTransform;
				transform.worldTransform = currentWorld;
				transform.isDirty = false;
			}

			if (m_registry.all_of<HierarchyComponent>(entity))
			{
				const auto& children = m_registry.get<HierarchyComponent>(entity).children;
				for (auto child : children)
				{
					if (m_registry.valid(child))
						self(self, child, currentWorld);
				}
			}
		};

		// Traverse all root entities (no parent)
		for (auto entity : m_registry.view<TransformComponent>())
		{
			bool isRoot = true;
			if (m_registry.all_of<HierarchyComponent>(entity))
			{
				auto parent = m_registry.get<HierarchyComponent>(entity).parent;
				if (parent != entt::null && m_registry.valid(parent))
					isRoot = false;
			}

			if (isRoot)
				updateEntityHierarchy(updateEntityHierarchy, entity, Matrix4::Identity);
		}
	}

	SceneObject Scene::LoadModel(const std::string& path, const bool preTransform)
	{
		const std::string filename = std::filesystem::path(path).filename().string();

		SceneObject root = CreateObject();
		root.AddComponent<NameComponent>().name = filename + "_root";
		root.AddComponent<TransformComponent>();

		const ResourceId modelId = m_resourceManager->LoadModel(path, preTransform);
		const ModelResource* model = m_resourceManager->GetModelData(modelId);

		auto& modelComp = root.AddComponent<ModelComponent>();
		modelComp.path = path;
		modelComp.preTransform = preTransform;

		if (!preTransform && !model->nodes.empty())
		{
			// Map node index to SceneObject
			std::vector<SceneObject> nodeObjects;
			nodeObjects.reserve(model->nodes.size());

			for (size_t i = 0; i < model->nodes.size(); ++i)
			{
				const auto& nodeRes = model->nodes[i];
				SceneObject nodeObj = CreateObject();
				nodeObj.AddComponent<NameComponent>().name = filename + "_" + nodeRes.name;

				auto& trans = nodeObj.AddComponent<TransformComponent>();
				trans.localTransform = nodeRes.transform;

				// For each mesh primitive in this node
				for (uint32_t meshIdx : nodeRes.meshIndices)
				{
					if (meshIdx < model->meshes.size())
					{
						const auto& meshRes = model->meshes[meshIdx];
						SceneObject meshObj = CreateObject();
						meshObj.AddComponent<NameComponent>().name = filename + "_mesh_" + std::to_string(meshIdx);
						meshObj.AddComponent<TransformComponent>(); // Identity local transform

						auto& meshComp = meshObj.AddComponent<MeshComponent>();
						meshComp.meshId = meshRes.id;
						meshComp.materialId = meshRes.materialId;
						meshComp.textureId = meshRes.textureId;

						meshObj.AddComponent<MaterialComponent>().materialId = meshRes.materialId;

						meshObj.SetParent(nodeObj);
						modelComp.children.push_back(meshObj);
					}
				}

				nodeObjects.push_back(nodeObj);
				modelComp.children.push_back(nodeObj);
			}

			// Wire up node hierarchy
			for (size_t i = 0; i < model->nodes.size(); ++i)
			{
				const auto& nodeRes = model->nodes[i];
				for (uint32_t childNodeIdx : nodeRes.children)
				{
					if (childNodeIdx < nodeObjects.size())
						nodeObjects[childNodeIdx].SetParent(nodeObjects[i]);
				}
			}

			// Attach root nodes to the model root
			for (uint32_t rootNodeIdx : model->rootNodes)
			{
				if (rootNodeIdx < nodeObjects.size())
					nodeObjects[rootNodeIdx].SetParent(root);
			}
		}
		else
		{
			modelComp.children.reserve(model->meshes.size());

			for (size_t i = 0; i < model->meshes.size(); ++i)
			{
				const auto& meshRes = model->meshes[i];
				SceneObject child = CreateObject();
				child.AddComponent<NameComponent>().name = filename + "_child_" + std::to_string(meshRes.id);
				child.AddComponent<TransformComponent>();

				auto& meshComp = child.AddComponent<MeshComponent>();
				meshComp.meshId = meshRes.id;
				meshComp.materialId = meshRes.materialId;
				meshComp.textureId = meshRes.textureId;

				child.AddComponent<MaterialComponent>().materialId = meshRes.materialId;
				child.SetParent(root);

				modelComp.children.push_back(child);
			}
		}

		UpdateTransforms();
		return root;
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
		if (index >= m_pointLights.size())
			throw std::out_of_range("No point light with that index");
		return m_pointLights.at(index);
	}
}
