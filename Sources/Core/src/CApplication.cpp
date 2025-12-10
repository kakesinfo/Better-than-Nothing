#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	void CApplication::Run()
	{
		CWindowManager::Initialize({"Better Than Nothing", 800, 600});

		VulkanContext vulkanContext;
		vulkanContext.vkSettings.bufferingMode = EVulkanBufferingMode::DoubleBuffering;
		vulkanContext.vkSettings.enableValidationLayers = true;
		vulkanContext.vkSettings.shadersFolderPath = "../../Assets/Shaders/";
		vulkanContext.vkSettings.enableVSync = true;

		const auto renderCache = std::make_unique<CRenderCache>(&vulkanContext);

		const auto renderList = std::make_unique<CRenderList>(renderCache.get());

		const auto renderer = std::make_unique<CRenderer>(&vulkanContext);

		renderer->SetRenderCache(renderCache.get());
		OnEnable(renderer.get());

		CWindowManager::GetWindow()->Show();

		bool shouldClose = false;
		while (!CWindowManager::GetWindow()->ShouldClose() && !shouldClose)
		{
			CWindow::PollEvents();

			if (CInputManager::IsKeyPressed(GLFW_KEY_ESCAPE))
				shouldClose = true;

			OnUpdate();

			OnRender(renderList.get());
			renderer->RenderFrame(renderList.get());

			renderList->Clear();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
