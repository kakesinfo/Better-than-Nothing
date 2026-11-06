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

#include "SceneObject_Components.hpp"
#include <entt/entt.hpp>

namespace BetterThanNothing
{

	class SceneObject
	{
	public:
		SceneObject() : m_registry(nullptr), m_entity(entt::null) {}

		explicit SceneObject(entt::registry* registry, entt::entity entity)
			: m_registry(registry), m_entity(entity)
		{}

		~SceneObject() = default;

		void SetName(const std::string_view& name)
		{
			if (IsValid())
				m_registry->emplace_or_replace<NameComponent>(m_entity, std::string(name));
		}

		[[nodiscard]] std::string_view GetName() const
		{
			if (IsValid())
			{
				if (const auto* nameComp = m_registry->try_get<NameComponent>(m_entity))
					return nameComp->name;
			}
			return {};
		}

		void SetParent(const SceneObject& parent)
		{
			if (!IsValid())
				return;

			if (m_registry->all_of<HierarchyComponent>(m_entity))
			{
				auto& currentHier = m_registry->get<HierarchyComponent>(m_entity);
				if (currentHier.parent != entt::null && m_registry->valid(currentHier.parent) && m_registry->all_of<HierarchyComponent>(currentHier.parent))
				{
					auto& parentChildren = m_registry->get<HierarchyComponent>(currentHier.parent).children;
					std::erase(parentChildren, m_entity);
				}
				currentHier.parent = entt::null;
			}

			if (parent.IsValid())
			{
				auto& childHier = m_registry->get_or_emplace<HierarchyComponent>(m_entity);
				childHier.parent = parent.GetEntity();

				auto& parentHier = parent.m_registry->get_or_emplace<HierarchyComponent>(parent.GetEntity());
				if (std::find(parentHier.children.begin(), parentHier.children.end(), m_entity) == parentHier.children.end())
				{
					parentHier.children.push_back(m_entity);
				}
			}

			if (m_registry->all_of<TransformComponent>(m_entity))
				m_registry->get<TransformComponent>(m_entity).isDirty = true;
		}

		[[nodiscard]] SceneObject GetParent() const
		{
			if (IsValid() && m_registry->all_of<HierarchyComponent>(m_entity))
			{
				auto parentEntity = m_registry->get<HierarchyComponent>(m_entity).parent;
				if (parentEntity != entt::null && m_registry->valid(parentEntity))
					return SceneObject(m_registry, parentEntity);
			}
			return SceneObject(nullptr, entt::null);
		}

		[[nodiscard]] std::vector<SceneObject> GetChildren() const
		{
			std::vector<SceneObject> result;
			if (IsValid() && m_registry->all_of<HierarchyComponent>(m_entity))
			{
				const auto& children = m_registry->get<HierarchyComponent>(m_entity).children;
				result.reserve(children.size());
				for (auto childEntity : children)
				{
					if (m_registry->valid(childEntity))
						result.emplace_back(m_registry, childEntity);
				}
			}
			return result;
		}

		void Destroy()
		{
			if (!IsValid())
				return;

			if (m_registry->all_of<HierarchyComponent>(m_entity))
			{
				auto childrenCopy = m_registry->get<HierarchyComponent>(m_entity).children;
				for (auto childEntity : childrenCopy)
				{
					if (m_registry->valid(childEntity))
						SceneObject(m_registry, childEntity).Destroy();
				}
			}

			SetParent(SceneObject(nullptr, entt::null));

			m_registry->destroy(m_entity);
			m_entity = entt::null;
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
		bool operator!=(const SceneObject& other) const { return !(*this == other); }

	private:
		entt::registry*	 m_registry;
		entt::entity	 m_entity;
	};
}
