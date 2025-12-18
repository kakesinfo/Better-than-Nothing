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
#include <Module_Vulkan/VulkanContext.hpp>
#include <Module_Engine/CEngine.hpp>
#include <Module_Engine/IEngineApplication.hpp>
#include <Module_Engine/CRenderContext.hpp>
#include <Module_Engine/CUpdateContext.hpp>
#include <Module_Platform/Manager/CWindowManager.hpp>
#include <Module_Platform/Manager/CInputManager.hpp>
#include <Module_Platform/CWindow.hpp>
#include <Module_Renderer/CRenderer.hpp>
#include <Module_Renderer/CRenderList.hpp>
#include <Module_Renderer/CRenderCache.hpp>
#include <Module_Renderer/CCamera.hpp>
#include <Module_Renderer/Data/CameraData.hpp>
#include <Module_Renderer/Data/DirectionalLightData.hpp>

namespace BetterThanNothing
{
	CEngine::CEngine(IEngineApplication* application): m_pApplication(application)
	{
		CWindowManager::Initialize({"Better Than Nothing", 1280, 720});

		m_pVulkanContext = std::make_unique<VulkanContext>();
		m_pVulkanContext->vkSettings.bufferingMode = EVulkanBufferingMode::DoubleBuffering;
		m_pVulkanContext->vkSettings.enableValidationLayers = true;
		m_pVulkanContext->vkSettings.shadersFolderPath = "../../../Assets/Shaders/";
		m_pVulkanContext->vkSettings.enableVSync = true;

		m_pRenderCache = std::make_unique<CRenderCache>(m_pVulkanContext.get());

		m_pRenderList = std::make_unique<CRenderList>(m_pRenderCache.get());

		m_pRenderer = std::make_unique<CRenderer>(m_pVulkanContext.get());

		m_pCamera = std::make_unique<CCamera>(45.0f, 0.1f, 1000.0f);

		m_pRenderer->AddPipeline([&](CVulkanPipelineBuilder* builder) {
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		}, m_pRenderCache.get()); // TODO: remove add pipeline from Renderer
	}

	void CEngine::_onUpdate(float deltaTime)
	{
		const CUpdateContext updateContext(m_pCamera.get(), deltaTime);
		m_pApplication->OnUpdate(&updateContext);
	}

	void CEngine::_onRender()
	{
		const CRenderContext renderContext(m_pRenderList.get());
		m_pApplication->OnRender(&renderContext);

		CameraData cameraData{};
		cameraData.view = m_pCamera->GetViewMatrix();
		cameraData.projection = m_pCamera->GetProjectionMatrix(m_pVulkanContext->vkSwapChain->GetAspectRatio());
		cameraData.position = m_pCamera->GetPosition();

		DirectionalLightData directionalLight{};
		directionalLight.position = glm::vec3(10.0f, 10.0f, 10.0f);
		directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);

		m_pRenderer->RenderFrame(m_pRenderList.get(), cameraData, directionalLight);
	}

	void CEngine::Run()
	{
		bool shouldClose = false;
		auto lastTime = std::chrono::high_resolution_clock::now();

		CWindowManager::GetWindow()->Show();

		m_pApplication->OnEnable();

		while (!CWindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = currentTime - lastTime;
			lastTime = currentTime;

			CWindow::PollEvents();

			_onUpdate(elapsed.count());
			_onRender();

			m_pRenderList->Clear();

			if (CInputManager::IsKeyPressed(EKey::Escape))
				shouldClose = true;

			CInputManager::ResetMousePosition();
		}


		m_pApplication->OnDisable();
		CWindowManager::Shutdown();
	}

	int BootEngine(IEngineApplication* app)
	{
		CEngine engine(app);
		engine.Run();
		return EXIT_SUCCESS;
	}
};
