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
		SetSize(200, 400);
	}

	static void DrawEntityNode(const SceneObject& obj)
	{
		std::string name = "SceneObject_" + std::to_string(static_cast<uint32_t>(obj.GetEntity()));
		if (obj.HasComponent<NameComponent>())
			name = obj.GetComponent<NameComponent>().name;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (!obj.HasComponent<ModelComponent>() && !obj.GetComponent<ModelComponent>().children.empty())
		{
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(obj.GetEntity())), flags, "%s", name.c_str());
		}
		else if (ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(obj.GetEntity())), flags, "%s", name.c_str()))
		{
			for (const auto& child : obj.GetComponent<ModelComponent>().children)
				DrawEntityNode(child);
			ImGui::TreePop();
		}
	}

	void ScenePanel::OnRender(Scene* scene)
	{
		std::unordered_set<entt::entity> childEntities;
		for (auto [entity, modelComp] : scene->GetView<ModelComponent>().each())
			for (const auto& child : modelComp.children)
				childEntities.insert(child.GetEntity());

		for (auto [entity, nameComp] : scene->GetView<NameComponent>().each())
		{
			if (childEntities.contains(entity))
				continue;

			SceneObject obj = scene->GetObject(entity);
			DrawEntityNode(obj);
		}
	}
}
