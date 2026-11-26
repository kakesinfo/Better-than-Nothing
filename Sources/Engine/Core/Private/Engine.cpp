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

#include <Common/EKey.hpp>
#include <Common/Metrics.hpp>
#include <Common/RenderSettings.hpp>
#include <Core/Engine.hpp>

#include "Platform/Platform.hpp"

#include <Core/IEngineApplication.hpp>
#include <Editor/DebugPanel.hpp>
#include <Editor/Editor.hpp>
#include <Editor/PerfPanel.hpp>
#include <Editor/ScenePanel.hpp>
#include <Platform/LegacyInputManager.hpp>
#include <Renderer/Renderer.hpp>
#include <RenderingDevice/RenderingDevice.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Camera.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneObject.hpp>
#include <Scene/SceneObject_Components.hpp>
#include <algorithm>
#include <chrono>
#include <format>
#include <thread>

namespace BetterThanNothing
{
	Engine::Engine(IEngineApplication* application) : m_application(application)
	{
		m_platform = Platform::Create("Better Than Nothing", 1280, 720);
		LegacyInputManager::Initialize();

		m_renderingDevice = RenderingDevice::Create({m_platform.get(), EBufferingMode::Double});
		m_resourceManager = ResourceManager::Create(m_renderingDevice.get());

#ifndef NDEBUG
		m_editor = Editor::Create(m_platform.get(), m_renderingDevice.get());
		m_editor->AddPanel<DebugPanel>();
		m_editor->AddPanel<ScenePanel>();
		m_editor->AddPanel<PerfPanel>();
#endif

		m_renderer = Renderer::Create(m_renderingDevice.get(), m_resourceManager.get(), m_editor.get());

		m_defaultScene = Scene::Create(m_resourceManager.get());
	}

	Engine::~Engine()
	{
		if (m_renderingDevice)
			m_renderingDevice->WaitIdle();

		m_defaultScene.reset();
		m_renderer.reset();
		m_editor.reset();
		m_resourceManager.reset();
		m_renderingDevice.reset();
		m_platform.reset();
	}

	void Engine::Run() const
	{
		bool shouldClose = false;
		auto lastTime = std::chrono::steady_clock::now();

		m_application->OnEnable(m_defaultScene.get());
		m_defaultScene->UpdateTransforms();

		m_platform->ShowWindow();

		while (m_platform->IsRunning() && !shouldClose)
		{
			Metrics::BeginRecording();

			const auto	currentTime = std::chrono::steady_clock::now();
			const float rawDeltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
			lastTime = currentTime;

			const float deltaTime = std::clamp(rawDeltaTime, 0.0f, 0.1f);

			Metrics::SetFrameTime(rawDeltaTime * 1000.0f);
			Metrics::SetFps(rawDeltaTime > 0.0f ? (1.0f / rawDeltaTime) : 0.0f);

			m_platform->PollEvents();

			if (LegacyInputManager::IsKeyPressed(EKey::Escape))
				shouldClose = true;

			m_application->OnUpdate(m_defaultScene.get(), deltaTime);
			m_defaultScene->Update(deltaTime);

			m_renderer->RenderFrame(m_defaultScene.get());

			LegacyInputManager::EndFrame();

			Metrics::EndRecording();

			const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - currentTime);
			const auto sleepDuration = RenderSettings::ComputeFrameSleepDuration(elapsed, RenderSettings::Get().targetFps);
			if (sleepDuration.count() > 0)
				std::this_thread::sleep_for(sleepDuration);
		}

		m_application->OnDisable();
	}

	int BootEngine(IEngineApplication* app)
	{
		const Engine engine(app);
		engine.Run();
		return EXIT_SUCCESS;
	}
};
