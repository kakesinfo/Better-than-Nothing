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
#include <concepts>
#include <entt/entt.hpp>
#include <string_view>

namespace BetterThanNothing
{
	class Scene;

	class SceneObject
	{
	public:
		SceneObject() = default;

		explicit SceneObject(Scene* scene, entt::registry* registry, const entt::entity entity)
			: m_scene(scene), m_registry(registry), m_entity(entity)
		{}

		~SceneObject() = default;

		void SetName(const std::string_view name)
		{
			if (!IsValid())
				return;
			m_registry->get_or_emplace<NameComponent>(m_entity).name = std::string(name);
		}

		[[nodiscard]] std::string_view GetName() const
		{
			if (const auto* nameComp = TryGetComponent<NameComponent>())
				return nameComp->name;
			return {};
		}

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
			requires std::derived_from<T, Component>
		{
			return m_registry->emplace<T>(m_entity, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args)
			requires std::derived_from<T, Component>
		{
			return m_registry->get_or_emplace<T>(m_entity, std::forward<Args>(args)...);
		}

		template <typename T>
		[[nodiscard]] T& GetComponent() const
			requires std::derived_from<T, Component>
		{
			return m_registry->get<T>(m_entity);
		}

		template <typename T>
		[[nodiscard]] T* TryGetComponent() const
			requires std::derived_from<T, Component>
		{
			return IsValid() ? m_registry->try_get<T>(m_entity) : nullptr;
		}

		template <typename T>
		[[nodiscard]] bool HasComponent() const
			requires std::derived_from<T, Component>
		{
			return IsValid() && m_registry->all_of<T>(m_entity);
		}

		template <typename T>
		void RemoveComponent() const
			requires std::derived_from<T, Component>
		{
			if (IsValid())
				m_registry->remove<T>(m_entity);
		}

		[[nodiscard]] TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }

		void					  SetParent(const SceneObject& parent) const;
		void					  RemoveParent() const;
		[[nodiscard]] SceneObject GetParent() const;
		[[nodiscard]] bool		  HasChildren() const;
		void					  Destroy();

		[[nodiscard]] bool IsValid() const { return m_registry != nullptr && m_entity != entt::null && m_registry->valid(m_entity); }
		explicit		   operator bool() const { return IsValid(); }

		[[nodiscard]] entt::entity GetEntity() const { return m_entity; }
		[[nodiscard]] Scene*	   GetScene() const { return m_scene; }

		bool operator==(const SceneObject& other) const { return m_registry == other.m_registry && m_entity == other.m_entity; }
		bool operator!=(const SceneObject& other) const { return !(*this == other); }

	private:
		Scene*			m_scene = nullptr;
		entt::registry* m_registry = nullptr;
		entt::entity	m_entity = entt::null;
	};
}
