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

#include <Common/EKey.hpp>
#include <Common/EMouseButton.hpp>
#include <Common/Logger.hpp>
#include <Core/EntryPoint.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Vector2.hpp>
#include <Math/Vector3.hpp>
#include <Platform/LegacyInputManager.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Terrain/TerrainConfig.hpp>
#include <Terrain/TerrainSystem.hpp>

#include <memory>

namespace BetterThanNothing
{
	static constexpr float kSunIlluminanceLux = 115000.0f; // Top-of-atmosphere sun illuminance (~115,000 lx)
	static constexpr float kPointLightLumens = 150000.0f;  // Strong architectural floodlights (~150,000 lm)
	static constexpr float kPointLightRadius = 25.0f;	   // Influence radius in meters

	void Runtime::OnEnable(Scene* scene)
	{
		scene->InitCamera(55.0f, 0.1f, 1000.0f);
		// Scenic golden-afternoon exposure: f/11, 1/125s, ISO 100 (EV100 ~= 13.89)
		scene->GetCamera()->SetPhysicalExposure(11.0f, 1.0f / 125.0f, 100.0f);
		// Overlook vantage showing Sponza, surrounding terrain, animated lakes, elm forest & volumetric god rays
		scene->GetCamera()->SetPosition(Vector3(44.0f, 16.0f, 34.0f));
		scene->GetCamera()->Rotate(42.0f, -15.0f);

		DirectionalLightData directionalLight{};
		directionalLight.revDirection = Vector3(0.55f, -0.40f, 0.72f).Normalized();
		directionalLight.color = Vector3(1.0f, 0.95f, 0.86f);
		directionalLight.illuminanceLux = kSunIlluminanceLux;

		scene->SetDirectionalLight(directionalLight);

		PointLightData redLight{};
		redLight.color = Vector3(1.0f, 0.15f, 0.1f);
		redLight.position = Vector3(15.0f, 1.5f, 5.0f);
		redLight.luminousPowerLm = kPointLightLumens;
		redLight.radius = kPointLightRadius;

		PointLightData greenLight{};
		greenLight.color = Vector3(0.15f, 1.0f, 0.2f);
		greenLight.position = Vector3(0.0f, 1.5f, -5.0f);
		greenLight.luminousPowerLm = kPointLightLumens;
		greenLight.radius = kPointLightRadius;

		PointLightData blueLight{};
		blueLight.color = Vector3(0.15f, 0.3f, 1.0f);
		blueLight.position = Vector3(-10.0f, 1.5f, 5.0f);
		blueLight.luminousPowerLm = kPointLightLumens;
		blueLight.radius = kPointLightRadius;

		scene->AddPointLight(redLight);
		scene->AddPointLight(greenLight);
		scene->AddPointLight(blueLight);

		// Build procedural chunked terrain + animated water lakes + scattered elm trees around Sponza
		m_terrainSystem = std::make_unique<TerrainSystem>(TerrainConfig::CreateSponzaLandscapeDefaults());
		m_terrainSystem->BuildTerrainAndWater(scene);
		m_terrainSystem->ScatterTrees(scene, TreeScatterConfig{});

		auto sponza = scene->LoadModel(".assets/sponza/sponza.obj", true);
		sponza.GetTransform().SetScale(0.01f);

		auto robot = scene->LoadModel(".assets/robot/robot.obj", true);
		robot.GetTransform().SetPosition(0.0f, 1.0f, 0.0f);
		robot.AddComponent<UpdateComponent>().onUpdate = [](SceneObject& obj, const float deltaTime) -> void {
			if (!obj.HasComponent<TransformComponent>())
				return;

			obj.GetTransform().Rotate(20.0f * deltaTime, Vector3::Up);
		};
	}

	void Runtime::OnUpdate(Scene* scene, const float deltaTime)
	{
		if (LegacyInputManager::IsMouseButtonPressed(EMouseButton::Left))
		{
			constexpr float rotateSpeed = 0.1f;
			const Vector2	delta = LegacyInputManager::GetMouseDelta();
			scene->GetCamera()->Rotate(-delta.GetX() * rotateSpeed, -delta.GetY() * rotateSpeed);
		}

		const float moveSpeed = 12.0f * deltaTime;

		auto deltaPos = Vector3::Zero;

		if (LegacyInputManager::IsKeyPressed(EKey::W))
			deltaPos += scene->GetCamera()->GetForward() * moveSpeed;
		if (LegacyInputManager::IsKeyPressed(EKey::S))
			deltaPos -= scene->GetCamera()->GetForward() * moveSpeed;
		if (LegacyInputManager::IsKeyPressed(EKey::D))
			deltaPos += scene->GetCamera()->GetRight() * moveSpeed;
		if (LegacyInputManager::IsKeyPressed(EKey::A))
			deltaPos -= scene->GetCamera()->GetRight() * moveSpeed;
		if (LegacyInputManager::IsKeyPressed(EKey::Space))
			deltaPos += Vector3::Up * moveSpeed;
		if (LegacyInputManager::IsKeyPressed(EKey::LeftShift))
			deltaPos -= Vector3::Up * moveSpeed;

		scene->GetCamera()->Move(deltaPos);
	}

	void Runtime::OnDisable()
	{
		m_terrainSystem.reset();
	}

	std::unique_ptr<IEngineApplication> CreateEngineApplication()
	{
		return std::make_unique<Runtime>();
	}
};
