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
#include <functional>
#include <vector>

namespace BetterThanNothing
{
	void SceneObject::SetParent(const SceneObject& parent) const
	{
		if (m_scene && IsValid())
			m_scene->SetParent(m_entity, parent.GetEntity());
	}

	void SceneObject::RemoveParent() const
	{
		if (m_scene && IsValid())
			m_scene->RemoveParent(m_entity);
	}

	SceneObject SceneObject::GetParent() const
	{
		if (!m_scene || !IsValid())
			return {};

		if (const auto* hierarchy = TryGetComponent<HierarchyComponent>(); hierarchy && hierarchy->parent != entt::null)
			return m_scene->GetObject(hierarchy->parent);

		return {};
	}

	bool SceneObject::HasChildren() const
	{
		if (const auto* hierarchy = TryGetComponent<HierarchyComponent>())
			return hierarchy->firstChild != entt::null;
		return false;
	}

	void SceneObject::Destroy()
	{
		if (m_scene && IsValid())
		{
			m_scene->DestroyEntity(m_entity);
			m_entity = entt::null;
		}
	}

	Scene::Scene(ResourceManager* resourceManager) : m_resourceManager(resourceManager)
	{
	}

	Scene::~Scene() = default;

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
			throw std::runtime_error("No camera initialized in Scene");
		return m_camera.get();
	}

	SceneObject Scene::CreateObject(const std::string_view name)
	{
		const entt::entity entity = m_registry.create();
		auto&			   nameComp = m_registry.emplace<NameComponent>(entity);
		nameComp.name = name.empty() ? ("Entity_" + std::to_string(static_cast<uint32_t>(entity))) : std::string(name);
		m_registry.emplace<TransformComponent>(entity);
		m_registry.emplace<HierarchyComponent>(entity);

		return GetObject(entity);
	}

	SceneObject Scene::GetObject(const entt::entity entity)
	{
		return SceneObject(this, &m_registry, entity);
	}

	void Scene::DestroyObject(const SceneObject& object)
	{
		if (object.IsValid())
			DestroyEntity(object.GetEntity());
	}

	void Scene::DestroyEntity(const entt::entity entity)
	{
		if (entity == entt::null || !m_registry.valid(entity))
			return;

		// Recursively destroy all children first
		if (const auto* hierarchy = m_registry.try_get<HierarchyComponent>(entity))
		{
			entt::entity child = hierarchy->firstChild;
			while (child != entt::null)
			{
				entt::entity nextSibling = entt::null;
				if (const auto* childHierarchy = m_registry.try_get<HierarchyComponent>(child))
					nextSibling = childHierarchy->nextSibling;

				DestroyEntity(child);
				child = nextSibling;
			}
		}

		DetachFromParent(entity);
		m_registry.destroy(entity);
	}

	bool Scene::IsDescendantOf(const entt::entity entity, const entt::entity potentialAncestor) const
	{
		if (entity == entt::null || potentialAncestor == entt::null)
			return false;

		entt::entity current = entity;
		while (current != entt::null && m_registry.valid(current))
		{
			if (current == potentialAncestor)
				return true;

			const auto* hierarchy = m_registry.try_get<HierarchyComponent>(current);
			if (!hierarchy)
				break;
			current = hierarchy->parent;
		}
		return false;
	}

	void Scene::DetachFromParent(const entt::entity child)
	{
		auto* childHierarchy = m_registry.try_get<HierarchyComponent>(child);
		if (!childHierarchy || childHierarchy->parent == entt::null)
			return;

		const entt::entity oldParent = childHierarchy->parent;
		if (m_registry.valid(oldParent))
		{
			if (auto* parentHierarchy = m_registry.try_get<HierarchyComponent>(oldParent))
			{
				if (parentHierarchy->firstChild == child)
					parentHierarchy->firstChild = childHierarchy->nextSibling;

				if (parentHierarchy->childCount > 0)
					--parentHierarchy->childCount;
			}
		}

		if (childHierarchy->prevSibling != entt::null && m_registry.valid(childHierarchy->prevSibling))
		{
			if (auto* prevHierarchy = m_registry.try_get<HierarchyComponent>(childHierarchy->prevSibling))
				prevHierarchy->nextSibling = childHierarchy->nextSibling;
		}

		if (childHierarchy->nextSibling != entt::null && m_registry.valid(childHierarchy->nextSibling))
		{
			if (auto* nextHierarchy = m_registry.try_get<HierarchyComponent>(childHierarchy->nextSibling))
				nextHierarchy->prevSibling = childHierarchy->prevSibling;
		}

		childHierarchy->parent = entt::null;
		childHierarchy->prevSibling = entt::null;
		childHierarchy->nextSibling = entt::null;
	}

	void Scene::SetParent(const entt::entity child, const entt::entity parent)
	{
		if (child == entt::null || !m_registry.valid(child))
			return;

		if (parent == entt::null)
		{
			RemoveParent(child);
			return;
		}

		if (!m_registry.valid(parent) || child == parent || IsDescendantOf(parent, child))
			return;

		auto& childHierarchy = m_registry.get_or_emplace<HierarchyComponent>(child);
		if (childHierarchy.parent == parent)
			return;

		DetachFromParent(child);

		auto& parentHierarchy = m_registry.get_or_emplace<HierarchyComponent>(parent);
		childHierarchy.parent = parent;
		childHierarchy.prevSibling = entt::null;
		childHierarchy.nextSibling = parentHierarchy.firstChild;

		if (parentHierarchy.firstChild != entt::null && m_registry.valid(parentHierarchy.firstChild))
		{
			if (auto* firstHierarchy = m_registry.try_get<HierarchyComponent>(parentHierarchy.firstChild))
				firstHierarchy->prevSibling = child;
		}

		parentHierarchy.firstChild = child;
		++parentHierarchy.childCount;

		if (auto* transform = m_registry.try_get<TransformComponent>(child))
			transform->MarkDirty();
	}

	void Scene::RemoveParent(const entt::entity child)
	{
		if (child == entt::null || !m_registry.valid(child))
			return;

		DetachFromParent(child);

		if (auto* transform = m_registry.try_get<TransformComponent>(child))
			transform->MarkDirty();
	}

	SceneObject Scene::LoadModel(const std::string& path, const bool preTransform)
	{
		const std::string	 filename = std::filesystem::path(path).filename().string();
		const ResourceId	 modelId = m_resourceManager->LoadModel(path, preTransform);
		const ModelResource* model = m_resourceManager->GetModelData(modelId);

		SceneObject root = CreateObject(filename);
		auto&		modelComp = root.AddComponent<ModelComponent>();
		modelComp.modelId = modelId;
		modelComp.path = path;
		modelComp.preTransform = preTransform;

		std::function<void(uint32_t, entt::entity)> spawnNode = [&](const uint32_t nodeIdx, const entt::entity parentEntity) {
			if (nodeIdx >= model->nodes.size())
				return;

			const auto&		  nodeRes = model->nodes[nodeIdx];
			const std::string nodeName = nodeRes.name.empty() ? (filename + "_node_" + std::to_string(nodeIdx)) : nodeRes.name;

			SceneObject nodeObj = CreateObject(nodeName);
			nodeObj.GetTransform().SetLocalMatrix(nodeRes.localTransform);
			SetParent(nodeObj.GetEntity(), parentEntity);

			if (nodeRes.meshIndices.size() == 1)
			{
				const uint32_t meshIdx = nodeRes.meshIndices[0];
				if (meshIdx < model->meshes.size())
				{
					const auto& meshRes = model->meshes[meshIdx];
					auto&		meshComp = nodeObj.AddComponent<MeshComponent>();
					meshComp.meshId = meshRes.id;
					meshComp.materialId = meshRes.materialId;
					meshComp.textureId = meshRes.textureId;
				}
			}
			else if (nodeRes.meshIndices.size() > 1)
			{
				for (size_t i = 0; i < nodeRes.meshIndices.size(); ++i)
				{
					const uint32_t meshIdx = nodeRes.meshIndices[i];
					if (meshIdx >= model->meshes.size())
						continue;

					const auto& meshRes = model->meshes[meshIdx];
					SceneObject primObj = CreateObject(nodeName + "_prim_" + std::to_string(i));
					SetParent(primObj.GetEntity(), nodeObj.GetEntity());

					auto& meshComp = primObj.AddComponent<MeshComponent>();
					meshComp.meshId = meshRes.id;
					meshComp.materialId = meshRes.materialId;
					meshComp.textureId = meshRes.textureId;
				}
			}

			for (const uint32_t childNodeIdx : nodeRes.children)
				spawnNode(childNodeIdx, nodeObj.GetEntity());
		};

		if (!model->rootNodes.empty())
		{
			for (const uint32_t rootNodeIdx : model->rootNodes)
				spawnNode(rootNodeIdx, root.GetEntity());
		}
		else
		{
			// Fallback for models without explicit node hierarchy
			for (size_t i = 0; i < model->meshes.size(); ++i)
			{
				const auto& meshRes = model->meshes[i];
				SceneObject child = CreateObject(filename + "_mesh_" + std::to_string(i));
				SetParent(child.GetEntity(), root.GetEntity());

				auto& meshComp = child.AddComponent<MeshComponent>();
				meshComp.meshId = meshRes.id;
				meshComp.materialId = meshRes.materialId;
				meshComp.textureId = meshRes.textureId;
			}
		}

		return root;
	}

	SceneObject Scene::SetDirectionalLight(const DirectionalLightData& data, const std::string_view name)
	{
		const auto view = m_registry.view<DirectionalLightComponent>();
		if (view.begin() != view.end())
		{
			const entt::entity entity = *view.begin();
			auto&			   light = view.get<DirectionalLightComponent>(entity);
			light.direction = data.revDirection;
			light.color = data.color;
			light.intensity = data.intensity;
			return GetObject(entity);
		}

		SceneObject obj = CreateObject(name);
		auto&		light = obj.AddComponent<DirectionalLightComponent>();
		light.direction = data.revDirection;
		light.color = data.color;
		light.intensity = data.intensity;
		return obj;
	}

	SceneObject Scene::AddPointLight(const PointLightData& data, const std::string_view name)
	{
		const std::string lightName = (name == "Point Light") ? ("Point Light " + std::to_string(++m_pointLightCounter)) : std::string(name);
		SceneObject		  obj = CreateObject(lightName);
		obj.GetTransform().SetPosition(data.position);

		auto& light = obj.AddComponent<PointLightComponent>();
		light.color = data.color;
		light.constant = data.constant;
		light.linear = data.linear;
		light.quadratic = data.quadratic;
		light.intensity = data.intensity;
		return obj;
	}

	void Scene::Update(const float deltaTime)
	{
		// Snapshot update entities so callbacks may safely spawn/destroy entities
		m_updateEntitiesScratch.clear();
		const auto updateView = m_registry.view<UpdateComponent>();
		m_updateEntitiesScratch.reserve(updateView.size());
		for (const entt::entity entity : updateView)
			m_updateEntitiesScratch.push_back(entity);

		for (const entt::entity entity : m_updateEntitiesScratch)
		{
			if (!m_registry.valid(entity))
				continue;

			auto* update = m_registry.try_get<UpdateComponent>(entity);
			if (update && update->onUpdate)
			{
				SceneObject obj = GetObject(entity);
				update->onUpdate(obj, deltaTime);
			}
		}

		UpdateTransforms();
	}

	void Scene::UpdateTransforms()
	{
		const auto view = m_registry.view<TransformComponent>();
		for (const entt::entity entity : view)
		{
			const auto* hierarchy = m_registry.try_get<HierarchyComponent>(entity);
			if (hierarchy && hierarchy->parent != entt::null)
				continue;

			UpdateTransformRecursive(entity, Matrix4::Identity, false);
		}
	}

	void Scene::UpdateTransformRecursive(const entt::entity entity, const Matrix4& parentWorldMatrix, const bool parentDirty)
	{
		auto* transform = m_registry.try_get<TransformComponent>(entity);
		if (!transform)
			return;

		const bool needsUpdate = transform->dirty || parentDirty;
		if (needsUpdate)
		{
			if (transform->dirty)
				transform->localMatrix = Matrix4::TRS(transform->position, transform->rotation, transform->scale);

			transform->worldMatrix = parentWorldMatrix * transform->localMatrix;
			transform->inverseWorldMatrix = transform->worldMatrix.Inverted();
			transform->dirty = false;
		}

		if (const auto* hierarchy = m_registry.try_get<HierarchyComponent>(entity))
		{
			entt::entity child = hierarchy->firstChild;
			while (child != entt::null && m_registry.valid(child))
			{
				entt::entity nextSibling = entt::null;
				if (const auto* childHierarchy = m_registry.try_get<HierarchyComponent>(child))
					nextSibling = childHierarchy->nextSibling;

				UpdateTransformRecursive(child, transform->worldMatrix, needsUpdate);
				child = nextSibling;
			}
		}
	}
}
