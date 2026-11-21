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

#include <Data/Graphics/DirectionalLightData.hpp>
#include <Data/Graphics/PointLightData.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace BetterThanNothing
{
	class ResourceManager;
	class Camera;
	class Matrix4;
	class SceneObject;

	class Scene
	{
	public:
		explicit Scene(ResourceManager* resourceManager);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		static std::unique_ptr<Scene> Create(ResourceManager* resourceManager);

		void				  InitCamera(float fov, float nearPlane, float farPlane);
		[[nodiscard]] Camera* GetCamera() const;
		[[nodiscard]] bool	  HasCamera() const { return m_camera != nullptr; }

		SceneObject CreateObject(std::string_view name = "");
		SceneObject GetObject(entt::entity entity);
		void		DestroyObject(const SceneObject& object);
		void		DestroyEntity(entt::entity entity);

		void			   SetParent(entt::entity child, entt::entity parent);
		void			   RemoveParent(entt::entity child);
		[[nodiscard]] bool IsDescendantOf(entt::entity entity, entt::entity potentialAncestor) const;

		SceneObject LoadModel(const std::string& path, bool preTransform = false);

		SceneObject SetDirectionalLight(const DirectionalLightData& data, std::string_view name = "Directional Light");
		SceneObject AddPointLight(const PointLightData& data, std::string_view name = "Point Light");

		void Update(float deltaTime);
		void UpdateTransforms();

		[[nodiscard]] entt::registry&		GetRegistry() { return m_registry; }
		[[nodiscard]] const entt::registry& GetRegistry() const { return m_registry; }
		[[nodiscard]] ResourceManager*		GetResourceManager() const { return m_resourceManager; }

		template <typename... Components>
		[[nodiscard]] auto GetView()
		{
			return m_registry.view<Components...>();
		}

		template <typename... Components>
		[[nodiscard]] auto GetView() const
		{
			return m_registry.view<Components...>();
		}

	private:
		void UpdateTransformRecursive(entt::entity entity, const Matrix4& parentWorldMatrix, bool parentDirty);
		void DetachFromParent(entt::entity child);

		ResourceManager*		  m_resourceManager = nullptr;
		entt::registry			  m_registry;
		std::unique_ptr<Camera>	  m_camera;
		uint32_t				  m_pointLightCounter = 0;
		std::vector<entt::entity> m_updateEntitiesScratch;
	};
}
