#include <module_renderer.hpp>
#include <module_vulkan.hpp>
#include <module_platform.hpp>

#include "Module_Engine/CApplication.hpp"

#include <glm/glm.hpp>

#include "Module_Engine/CRenderContext.hpp"
#include "Module_Engine/CUpdateContext.hpp"

namespace BetterThanNothing
{
	void CApplication::Run()
	{
		CWindowManager::Initialize({"Better Than Nothing", 1280, 720});

		VulkanContext vulkanContext;
		vulkanContext.vkSettings.bufferingMode = EVulkanBufferingMode::DoubleBuffering;
		vulkanContext.vkSettings.enableValidationLayers = true;
		vulkanContext.vkSettings.shadersFolderPath = "../../../Assets/Shaders/";
		vulkanContext.vkSettings.enableVSync = true;

		const auto renderCache = std::make_unique<CRenderCache>(&vulkanContext);

		const auto renderList = std::make_unique<CRenderList>(renderCache.get());

		const auto renderer = std::make_unique<CRenderer>(&vulkanContext);

		const auto camera = std::make_shared<CCamera>(45.0f, 0.1f, 1000.0f);

		renderer->SetRenderCache(renderCache.get());

		renderer->AddPipeline([&](CVulkanPipelineBuilder* builder) {
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		});

		OnEnable();

		bool shouldClose = false;
		bool firstPass = true;
		auto lastTime = std::chrono::high_resolution_clock::now();

		while (!CWindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = currentTime - lastTime;
			lastTime = currentTime;

			CWindow::PollEvents();

			if (CInputManager::IsKeyPressed(GLFW_KEY_ESCAPE))
				shouldClose = true;

			CUpdateContext updateContext(camera.get(), elapsed.count());
			OnUpdate(&updateContext);

			{
				CameraData cameraData{};
				cameraData.view = camera->GetViewMatrix();
				cameraData.projection = camera->GetProjectionMatrix(vulkanContext.vkSwapChain->GetAspectRatio());
				cameraData.position = camera->GetPosition();

				DirectionalLightData directionalLight{};
				directionalLight.position = glm::vec3(10.0f, 10.0f, 10.0f);
				directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);

				CRenderContext renderContext(renderList.get());
				OnRender(&renderContext);

				renderer->RenderFrame(renderList.get(), cameraData, directionalLight);
				renderList->Clear();
			}

			CInputManager::ResetMousePosition();

			if (firstPass) {
				firstPass = false;
				CWindowManager::GetWindow()->Show();
			}
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
