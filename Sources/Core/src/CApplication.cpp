#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	void CApplication::Run()
	{
		CWindowManager::Initialize({"Better Than Nothing", 1280, 720});

		VulkanContext vulkanContext;
		vulkanContext.vkSettings.bufferingMode = EVulkanBufferingMode::DoubleBuffering;
		vulkanContext.vkSettings.enableValidationLayers = true;
		vulkanContext.vkSettings.shadersFolderPath = "../../Assets/Shaders/";
		vulkanContext.vkSettings.enableVSync = true;

		const auto renderCache = std::make_unique<CRenderCache>(&vulkanContext);

		const auto renderList = std::make_unique<CRenderList>(renderCache.get());

		const auto renderer = std::make_unique<CRenderer>(&vulkanContext);

		const auto camera = std::make_shared<CCamera>(45.0f, 0.1f, 1000.0f);

		renderer->SetRenderCache(renderCache.get());
		OnEnable(renderer.get());

		CWindowManager::GetWindow()->Show();

		auto lastTime = std::chrono::high_resolution_clock::now();

		bool shouldClose = false;
		while (!CWindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = currentTime - lastTime;
			lastTime = currentTime;

			CWindow::PollEvents();

			if (CInputManager::IsKeyPressed(GLFW_KEY_ESCAPE))
				shouldClose = true;

			OnUpdate(camera.get(), elapsed.count());

			{
				OnRender(renderList.get());
				renderer->RenderFrame(renderList.get(), camera.get());
				renderList->Clear();
			}

			CInputManager::ResetMousePosition();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
