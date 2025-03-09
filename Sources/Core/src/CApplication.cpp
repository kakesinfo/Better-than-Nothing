#include "BetterThanNothing.hpp"
#include "Renderer/CShaderPool.hpp"

namespace BetterThanNothing
{
	void CApplication::Run()
	{
		CApplicationContext context = ApplicationContextBuilder()
			.SetWindowTitle("Better Than Nothing")
			.SetWindowSize(800, 600)
			.SetWindowResizable(true)
			.EnableValidationLayers(true)
			.SetShadersFolderPath("../../Assets/Shaders/")
			.SetBufferingMode(EVulkanBufferingMode::DoubleBuffering)
			.EnableVSync(true)
			.Build();

		context.Initialize();

		OnEnable(&context);

		context.GetWindow()->Show();

		while (!context.GetWindow()->ShouldClose())
		{
			CWindow::PollEvents();

			OnUpdate(&context);
			OnRender(&context);

			context.GetRenderer()->Render();

			std::this_thread::sleep_for(std::chrono::milliseconds (10));
		}

		context.GetVulkanDevice()->WaitIdle();

		OnDisable(&context);
	}
};
