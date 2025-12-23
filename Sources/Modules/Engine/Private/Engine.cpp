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

#include <module_vulkan.hpp>

#include <Module_Base/EKey.hpp>
#include <Module_Engine/Engine.hpp>

#include "Module_Math/Matrix4.hpp"

#include <Module_Engine/IEngineApplication.hpp>
#include <Module_Engine/RenderContext.hpp>
#include <Module_Engine/UpdateContext.hpp>
#include <Module_Platform/Manager/InputManager.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <Module_Renderer/Camera.hpp>
#include <Module_Renderer/Data/CameraData.hpp>
#include <Module_Renderer/Data/DirectionalLightData.hpp>
#include <Module_Renderer/RenderCache.hpp>
#include <Module_Renderer/RenderList.hpp>
#include <Module_Renderer/Renderer.hpp>
#include <Module_Vulkan/VulkanContext.hpp>

#include "Module_Renderer/RenderMetrics.hpp"

namespace BetterThanNothing
{
	Engine::Engine(IEngineApplication* application) : m_application(application)
	{
		WindowManager::Initialize({"Better Than Nothing", 1280, 720});

		m_context = std::make_unique<VulkanContext>();
		m_context->vkSettings.bufferingMode = EVulkanBufferingMode::DoubleBuffering;
		m_context->vkSettings.enableValidationLayers = true;
		m_context->vkSettings.shadersFolderPath = "../../../Assets/Shaders/";
		m_context->vkSettings.enableVSync = true;

		m_renderCache = std::make_unique<RenderCache>(m_context.get());
		m_renderList = std::make_unique<RenderList>(m_renderCache.get());
		m_renderer = std::make_unique<Renderer>(m_context.get());
		m_camera = std::make_unique<Camera>(45.0f, 0.1f, 1000.0f);

		m_renderer->AddPipeline(
			[&](VulkanPipelineBuilder* builder) {
				builder->SetName("main");
				builder->AddVertexShader("main/main.vert");
				builder->AddFragmentShader("main/main.frag");
			},
			m_renderCache.get()); // TODO: remove add pipeline from Renderer
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
		cameraData.projection = m_camera->GetProjectionMatrix(m_context->vkSwapChain->GetAspectRatio());
		cameraData.position = m_camera->GetPosition();

		DirectionalLightData directionalLight{};
		directionalLight.position = Vector3(10.0f, 10.0f, 10.0f);
		directionalLight.color = Vector3(1.0f, 1.0f, 1.0f);

		m_renderer->RenderFrame(m_renderList.get(), cameraData, directionalLight);
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

			Window::PollEvents();

			OnUpdate(elapsed.count());
			OnRender();

			m_renderList->Clear();

			if (InputManager::IsKeyPressed(EKey::Escape))
				shouldClose = true;

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
