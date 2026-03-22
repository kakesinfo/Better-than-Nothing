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

#include "Runtime.hpp"

#include "../../Modules/Platform/Public/Module_Platform/Manager/InputManager.hpp"
#include "Module_Base/Logger.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneObject.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>
#include <Module_Engine/EntryPoint.hpp>
#include <Module_Math/Matrix4.hpp>
#include <Module_Math/Vector2.hpp>
#include <Module_Math/Vector3.hpp>

#include <memory>

namespace BetterThanNothing
{
	static constexpr float kConstant = 1.0f;
	static constexpr float kLinear = 0.09f;
	static constexpr float kQuadratic = 0.032f;
	static constexpr float kIntensity = 10.0f;

	void Runtime::OnEnable(Scene* scene)
	{
		scene->InitCamera(50.0f, 0.1f, 1000.0f);

		DirectionalLightData directionalLight{};
		directionalLight.revDirection = Vector3(1.0f, -4.0f, -2.0f).Normalized();
		directionalLight.color = Vector3(1.0f, 0.95f, 0.8f);
		directionalLight.intensity = 5.0f;

		scene->SetDirectionalLight(directionalLight);

		PointLightData redLight{};
		redLight.color = Vector3(1.0f, 0.0f, 0.0f);
		redLight.position = Vector3(15.0f, 0.0f, 5.0f);
		redLight.constant = kConstant;
		redLight.linear = kLinear;
		redLight.quadratic = kQuadratic;
		redLight.intensity = kIntensity;

		PointLightData greenLight{};
		greenLight.color = Vector3(0.0f, 1.0f, 0.0f);
		greenLight.position = Vector3(0.0f, 0.0f, -5.0f);
		greenLight.constant = kConstant;
		greenLight.linear = kLinear;
		greenLight.quadratic = kQuadratic;
		greenLight.intensity = kIntensity;

		PointLightData blueLight{};
		blueLight.color = Vector3(0.0f, 0.0f, 1.0f);
		blueLight.position = Vector3(-10.0f, 0.0f, 5.0f);
		blueLight.constant = kConstant;
		blueLight.linear = kLinear;
		blueLight.quadratic = kQuadratic;
		blueLight.intensity = kIntensity;

		scene->AddPointLight(redLight);
		scene->AddPointLight(greenLight);
		scene->AddPointLight(blueLight);

		auto sponza = scene->CreateObject();
		sponza.AddComponent<TransformComponent>().localTransform = Matrix4::Identity.Translate(0.0f, 1.0f, 0.0f).Scale(0.01f);
		sponza.LoadModel("../../../.assets/sponza/sponza.obj", true);

		auto robot = scene->CreateObject();
		robot.AddComponent<TransformComponent>().localTransform = Matrix4::Identity.Translate(0.0f, 1.0f, 0.0f);
		robot.LoadModel("../../../.assets/robot/robot.obj", true);
		robot.AddComponent<UpdateComponent>().onUpdate = [](const SceneObject& obj, const float deltaTime) -> void {
			if (!obj.HasComponent<TransformComponent>())
				return;

			auto& transform = obj.GetComponent<TransformComponent>();
			transform.localTransform = transform.localTransform.Rotate(20.0f * deltaTime, Vector3::Up);
		};
	}

	void Runtime::OnUpdate(Scene* scene, const float deltaTime)
	{
		if (InputManager::IsMouseButtonPressed(EMouseButton::Left))
		{
			constexpr float rotateSpeed = 0.1f;
			const Vector2	delta = InputManager::GetMouseDelta();
			scene->GetCamera()->Rotate(-delta.GetX() * rotateSpeed, -delta.GetY() * rotateSpeed);
		}

		const float moveSpeed = 5.0f * deltaTime;

		auto deltaPos = Vector3::Zero;

		if (InputManager::IsKeyPressed(EKey::W))
			deltaPos += scene->GetCamera()->GetForward() * moveSpeed;
		if (InputManager::IsKeyPressed(EKey::S))
			deltaPos -= scene->GetCamera()->GetForward() * moveSpeed;
		if (InputManager::IsKeyPressed(EKey::D))
			deltaPos += scene->GetCamera()->GetRight() * moveSpeed;
		if (InputManager::IsKeyPressed(EKey::A))
			deltaPos -= scene->GetCamera()->GetRight() * moveSpeed;
		if (InputManager::IsKeyPressed(EKey::Space))
			deltaPos += Vector3::Up * moveSpeed;
		if (InputManager::IsKeyPressed(EKey::LeftShift))
			deltaPos -= Vector3::Up * moveSpeed;

		scene->GetCamera()->Move(deltaPos);
	}

	void Runtime::OnDisable() {}

	std::unique_ptr<IEngineApplication> CreateEngineApplication()
	{
		return std::make_unique<Runtime>();
	}
};
