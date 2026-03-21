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

#include "ResourceManager/ResourceManager.hpp"
#include "SceneObject_Components.hpp"

#include <entt/entt.hpp>

namespace BetterThanNothing
{

	class SceneObject
	{
	public:
		SceneObject() : m_resourceManager(nullptr), m_registry(nullptr), m_entity(entt::null) {}

		explicit SceneObject(ResourceManager* resourceManager, entt::registry* registry, entt::entity entity)
			: m_resourceManager(resourceManager), m_registry(registry), m_entity(entity)
		{}

		~SceneObject() = default;

		void LoadModel(const std::string& path, const bool preTransform = false)
		{
			auto& modelComp = AddComponent<ModelComponent>();
			modelComp.path = path;

			const ResourceId modelId = m_resourceManager->LoadModel(path, preTransform);
			const ModelResource* model = m_resourceManager->GetModel(modelId);

			modelComp.meshes.resize(model->meshes.size());
			for (const auto& mesh : model->meshes)
				modelComp.meshes.push_back(mesh);
		}

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
			requires std::derived_from<T, Component>
		{
			return m_registry->emplace<T>(m_entity, std::forward<Args>(args)...);
		}

		template <typename T>
		[[nodiscard]] T& GetComponent() const
			requires std::derived_from<T, Component>
		{
			return m_registry->get<T>(m_entity);
		}

		template <typename T>
		[[nodiscard]] bool HasComponent() const
			requires std::derived_from<T, Component>
		{
			return m_registry->all_of<T>(m_entity);
		}

		template <typename T>
		void RemoveComponent() const
			requires std::derived_from<T, Component>
		{
			m_registry->remove<T>(m_entity);
		}

		[[nodiscard]] bool IsValid() const { return m_registry != nullptr && m_registry->valid(m_entity); }
		explicit		   operator bool() const { return IsValid(); }

		[[nodiscard]] entt::entity GetEntity() const { return m_entity; }

		bool operator==(const SceneObject& other) const { return m_registry == other.m_registry && m_entity == other.m_entity; }
		bool operator!=(const SceneObject& other) const { return m_registry == other.m_registry || m_entity != other.m_entity; }

	private:
		ResourceManager* m_resourceManager;
		entt::registry*	 m_registry;
		entt::entity	 m_entity;
	};
}
