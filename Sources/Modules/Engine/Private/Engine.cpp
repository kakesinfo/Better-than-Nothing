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

#include <Editor/DebugPanel.hpp>
#include <Editor/Editor.hpp>
#include <Module_Base/EKey.hpp>
#include <Module_Base/Metrics.hpp>
#include <Module_Engine/Engine.hpp>

#include "Editor/PerfPanel.hpp"
#include "Editor/ScenePanel.hpp"

#include <Module_Engine/IEngineApplication.hpp>
#include <Module_Platform/Manager/InputManager.hpp>
#include <Module_Platform/Manager/WindowManager.hpp>
#include <Module_Platform/Window.hpp>
#include <Renderer/Renderer.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <chrono>
#include <format>

namespace BetterThanNothing
{
	Engine::Engine(IEngineApplication* application) : m_application(application)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
			throw std::runtime_error(std::format("Cannot init SDL: {}", SDL_GetError()));

		WindowManager::Initialize({"Better Than Nothing", 1280, 720});
		InputManager::Initialize();

		m_renderingDevice = RenderingDevice::Create({EBufferingMode::Double});
		m_resourceManager = ResourceManager::Create(m_renderingDevice.get());


#ifndef NDEBUG
		m_editor = Editor::Create(WindowManager::GetWindow().get(), m_renderingDevice.get());
		m_editor->AddPanel<DebugPanel>();
		m_editor->AddPanel<ScenePanel>();
		m_editor->AddPanel<PerfPanel>();
#endif

		m_renderer = Renderer::Create(m_renderingDevice.get(), m_resourceManager.get(), m_editor.get());

		m_defaultScene = Scene::Create(m_resourceManager.get());
	}

	void Engine::OnUpdate(const float deltaTime) const
	{
		m_application->OnUpdate(m_defaultScene.get(), deltaTime);

		const auto sceneObjects = m_defaultScene->GetView<UpdateComponent>();
		for (auto [entity, update] : sceneObjects.each())
		{
			auto sceneObject = m_defaultScene->GetObject(entity);
			update.onUpdate(sceneObject, deltaTime);
		}
	}

	void Engine::Run() const
	{
		bool shouldClose = false;
		auto lastTime = std::chrono::high_resolution_clock::now();

		m_application->OnEnable(m_defaultScene.get());

		WindowManager::GetWindow()->Show();

		while (!WindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			Metrics::BeginRecording();

			const auto currentTime = std::chrono::high_resolution_clock::now();
			const auto elapsedCount = static_cast<std::chrono::duration<float>>(currentTime - lastTime).count();
			lastTime = currentTime;

			Metrics::SetFrameTime(elapsedCount * 1000.0f);
			Metrics::SetFps(1.0f / elapsedCount);

			WindowManager::GetWindow()->PollEvents();

			if (InputManager::IsKeyPressed(EKey::Escape))
				shouldClose = true;

			OnUpdate(elapsedCount);
			m_renderer->RenderFrame(m_defaultScene.get());

			InputManager::ResetMousePosition();

			Metrics::EndRecording();
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
