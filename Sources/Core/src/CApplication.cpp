#include "BetterThanNothing.hpp"
#include "Manager/CWindowManager.hpp"
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

		const auto renderList = std::make_unique<CRenderList>();

		while (!CWindowManager::GetWindow()->ShouldClose())
		{
			CWindow::PollEvents();

			OnUpdate();

			OnRender(renderList.get()); // TODO: use a "Pencil" class to record rendering commands
			renderer->RenderFrame(renderList.get());

			renderList->Clear();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		OnDisable();

		CWindowManager::Shutdown();
	}
};
