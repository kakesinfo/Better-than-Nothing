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

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING

#include "Editor/ScenePanel.hpp"
#include <Editor/Panel.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <imgui.h>

namespace BetterThanNothing
{
	ScenePanel::ScenePanel() : Panel("Scene Panel")
	{
		SetPosition(0, 250);
		SetSize(280, 450);
	}

	void ScenePanel::DrawEntityNode(Scene* scene, const entt::entity entity)
	{
		const SceneObject obj = scene->GetObject(entity);
		if (!obj.IsValid())
			return;

		std::string name;
		if (const auto* nameComp = obj.TryGetComponent<NameComponent>(); nameComp && !nameComp->name.empty())
			name = nameComp->name;
		else
			name = "Entity_" + std::to_string(static_cast<uint32_t>(entity));

		const auto* hierarchy = obj.TryGetComponent<HierarchyComponent>();
		const bool	hasChildren = hierarchy && hierarchy->firstChild != entt::null;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (m_selectedEntity == entity)
			flags |= ImGuiTreeNodeFlags_Selected;
		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		const bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uintptr_t>(entity)), flags, "%s", name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			m_selectedEntity = entity;

		if (hasChildren && opened)
		{
			entt::entity child = hierarchy->firstChild;
			while (child != entt::null)
			{
				entt::entity nextSibling = entt::null;
				if (const auto* childHierarchy = scene->GetRegistry().try_get<HierarchyComponent>(child))
					nextSibling = childHierarchy->nextSibling;

				DrawEntityNode(scene, child);
				child = nextSibling;
			}
			ImGui::TreePop();
		}
	}

	void ScenePanel::DrawInspector(Scene* scene)
	{
		if (m_selectedEntity == entt::null || !scene->GetRegistry().valid(m_selectedEntity))
		{
			m_selectedEntity = entt::null;
			return;
		}

		const SceneObject obj = scene->GetObject(m_selectedEntity);

		ImGui::SeparatorText("Inspector");
		if (const auto* nameComp = obj.TryGetComponent<NameComponent>())
			ImGui::Text("Name: %s", nameComp->name.c_str());

		if (auto* transform = obj.TryGetComponent<TransformComponent>())
		{
			float pos[3] = {transform->position.GetX(), transform->position.GetY(), transform->position.GetZ()};
			if (ImGui::DragFloat3("Position", pos, 0.05f))
				transform->SetPosition(pos[0], pos[1], pos[2]);

			float scale[3] = {transform->scale.GetX(), transform->scale.GetY(), transform->scale.GetZ()};
			if (ImGui::DragFloat3("Scale", scale, 0.01f))
				transform->SetScale(Vector3(scale[0], scale[1], scale[2]));
		}

		if (auto* dirLight = obj.TryGetComponent<DirectionalLightComponent>())
		{
			ImGui::SeparatorText("Directional Light");
			float dir[3] = {dirLight->direction.GetX(), dirLight->direction.GetY(), dirLight->direction.GetZ()};
			if (ImGui::DragFloat3("Direction", dir, 0.02f))
				dirLight->direction = Vector3(dir[0], dir[1], dir[2]).Normalized();

			float color[3] = {dirLight->color.GetX(), dirLight->color.GetY(), dirLight->color.GetZ()};
			if (ImGui::ColorEdit3("Color", color))
				dirLight->color = Vector3(color[0], color[1], color[2]);

			ImGui::DragFloat("Illuminance (lux)", &dirLight->illuminanceLux, 500.0f, 0.0f, 200000.0f);
		}

		if (auto* pointLight = obj.TryGetComponent<PointLightComponent>())
		{
			ImGui::SeparatorText("Point Light");
			float color[3] = {pointLight->color.GetX(), pointLight->color.GetY(), pointLight->color.GetZ()};
			if (ImGui::ColorEdit3("Color", color))
				pointLight->color = Vector3(color[0], color[1], color[2]);

			ImGui::DragFloat("Luminous Power (lm)", &pointLight->luminousPowerLm, 500.0f, 0.0f, 500000.0f);
			ImGui::DragFloat("Radius (m)", &pointLight->radius, 0.5f, 0.1f, 200.0f);
		}

		if (const auto* meshComp = obj.TryGetComponent<MeshComponent>())
		{
			ImGui::SeparatorText("Mesh");
			ImGui::Text("Mesh ID: %llu", static_cast<unsigned long long>(meshComp->meshId));
			ImGui::Text("Material ID: %llu", static_cast<unsigned long long>(meshComp->materialId));
			ImGui::Text("Texture ID: %llu", static_cast<unsigned long long>(meshComp->textureId));
		}
	}

	void ScenePanel::OnRender(Scene* scene)
	{
		if (!scene)
			return;

		ImGui::SeparatorText("Hierarchy");
		const float hierarchyHeight = m_selectedEntity != entt::null ? ImGui::GetContentRegionAvail().y * 0.55f : 0.0f;
		if (ImGui::BeginChild("SceneHierarchy", ImVec2(0.0f, hierarchyHeight), false))
		{
			for (const entt::entity entity : scene->GetView<NameComponent>())
			{
				const auto* hierarchy = scene->GetRegistry().try_get<HierarchyComponent>(entity);
				if (hierarchy && hierarchy->parent != entt::null)
					continue;

				DrawEntityNode(scene, entity);
			}
		}
		ImGui::EndChild();

		DrawInspector(scene);
	}
}
