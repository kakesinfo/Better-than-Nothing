#include "BetterThanNothing.hpp"
#include "Manager/CWIndowManager.hpp"
#include "Renderer/CRendererBuilder.hpp"
#include "Renderer/CShaderPool.hpp"

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

		const auto pRenderer = renderer.get();
		OnEnable(pRenderer);

		CWindowManager::GetWindow()->Show();

		while (!CWindowManager::GetWindow()->ShouldClose())
		{
			CWindow::PollEvents();

			OnUpdate();
			OnRender(pRenderer);

			renderer->RenderFrame();

			std::this_thread::sleep_for(std::chrono::milliseconds (1));
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
