/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
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

#include <RenderingApi.hpp>

#include <Module_Base/EKey.hpp>
#include <Module_Engine/Engine.hpp>

#include "Module_Math/Matrix4.hpp"
#include "ResourceManager/ResourceManager.hpp"

#include <Module_Data/Graphics/CameraData.hpp>
#include <Module_Data/Graphics/DirectionalLightData.hpp>
#include <Module_Engine/IEngineApplication.hpp>
#include <Module_Engine/RenderContext.hpp>
#include <Module_Engine/UpdateContext.hpp>
#include <Module_Platform/Manager/InputManager.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/RenderList.hpp>
#include <Renderer/Renderer.hpp>
#include <RenderingApi/Old/VulkanContext.hpp>

#include "Renderer/RenderMetrics.hpp"

namespace BetterThanNothing
{
	Engine::Engine(IEngineApplication* application) : m_application(application)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
			throw std::runtime_error(std::format("Cannot init SDL: {}", SDL_GetError()));

		WindowManager::Initialize({"Better Than Nothing", 1280, 720});
		InputManager::Initialize();

		m_context = std::make_unique<VulkanContext>();
		m_resourceManager = std::make_unique<ResourceManager>(m_context.get());
		m_renderList = std::make_unique<RenderList>(m_resourceManager.get());
		m_legacyRenderer = std::make_unique<Renderer>(m_context.get(), m_resourceManager.get());
		m_camera = std::make_unique<Camera>(45.0f, 0.1f, 1000.0f);

		m_legacyRenderer->AddPipeline([&](VulkanPipelineBuilder* builder) {
			builder->SetPath("../../../Assets/Shaders/");
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		});

		DirectionalLightData directionalLight{};
		directionalLight.revDirection = Vector3(1.0f, -4.0f, -2.0f).Normalized();
		directionalLight.color = Vector3(1.0f, 0.95f, 0.8f);
		directionalLight.intensity = 5.0f;

		m_renderList->SetDirectionalLight(directionalLight);

		float kConstant = 1.0f;
		float kLinear = 0.09f;
		float kQuadratic = 0.032f;
		float kIntensity = 10.0f;

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

		m_renderList->AddPointLight(redLight);
		m_renderList->AddPointLight(greenLight);
		m_renderList->AddPointLight(blueLight);
	}

	void Engine::OnUpdate(const float deltaTime) const
	{
		const UpdateContext updateContext(m_camera.get(), deltaTime);
		m_application->OnUpdate(&updateContext);
	}

	void Engine::OnRender() const
	{
		const RenderContext renderContext(m_renderList.get());
		m_application->OnRender(&renderContext);

		CameraData cameraData{};
		cameraData.view = m_camera->GetViewMatrix();
		cameraData.projection = m_camera->GetProjectionMatrix(m_context->swapChain->GetAspectRatio());
		cameraData.position = m_camera->GetPosition();
		cameraData.exposure = 0.5f;

		m_legacyRenderer->RenderFrame(m_renderList.get(), cameraData);
	}

	void Engine::Run() const
	{
		bool shouldClose = false;
		auto lastTime = std::chrono::high_resolution_clock::now();

		m_application->OnEnable();

		WindowManager::GetWindow()->Show();

		while (!WindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			RenderMetrics::BeginRecording();

			const auto					 currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = currentTime - lastTime;
			lastTime = currentTime;

			RenderMetrics::SetFrameTime(elapsed.count() * 1000.0f);
			RenderMetrics::SetFps(1.0f / elapsed.count());

			WindowManager::GetWindow()->PollEvents();

			if (InputManager::IsKeyPressed(EKey::Escape))
				shouldClose = true;

			OnUpdate(elapsed.count());
			OnRender();

			m_renderList->Clear();

			InputManager::ResetMousePosition();

			RenderMetrics::EndRecording();
		}

		m_application->OnDisable();
		WindowManager::Shutdown();
	}

	int BootEngine(IEngineApplication* app)
	{
		const Engine engine(app);
		engine.Run();
		return EXIT_SUCCESS;
	}
};
