#include "BetterThanNothing.hpp"
#include "Manager/CWIndowManager.hpp"
#include "Renderer/CRendererBuilder.hpp"

namespace BetterThanNothing
{
	void CApplication::Run()
	{
		CWindowManager::Initialize({
			"Better Than Nothing", 800, 600, false, true
		});

		const auto renderer = CRendererBuilder()
			.EnableValidationLayers(true)
			.SetShadersFolderPath("../../Assets/Shaders/")
			.SetBufferingMode(EVulkanBufferingMode::DoubleBuffering)
			.EnableVSync(true)
			.Build();

		OnEnable(renderer.get());

		CWindowManager::GetWindow()->Show();

		const auto renderQueue = std::make_unique<CRenderQueue>();

		while (!CWindowManager::GetWindow()->ShouldClose())
		{
			CWindow::PollEvents();

			OnUpdate();

			OnRender(renderQueue.get()); // TODO: pass renderQueue instead of renderer
			renderer->RenderFrame(renderQueue.get()); // TODO: pass renderQueue

			renderQueue->Clear();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
