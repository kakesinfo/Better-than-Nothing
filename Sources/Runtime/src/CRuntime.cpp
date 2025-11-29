#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	void CRuntime::OnEnable(IRenderer* renderer)
	{
		renderer->AddPipeline([&](VulkanPipelineBuilder& builder) {
			builder.SetName("main");
			builder.AddVertexShader("main/main.vert");
			builder.AddFragmentShader("main/main.frag");
		});

		mesh.AddVertex({-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f});
		mesh.AddVertex({0.5f, -0.5f}, {0.0f, 1.0f, 0.0f});
		mesh.AddVertex({0.5f, 0.5f}, {0.0f, 0.0f, 1.0f});
		mesh.AddVertex({-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f});
		mesh.AddIndices({0, 1, 2, 2, 3, 0});
		mesh.Build();
	}

	void CRuntime::OnUpdate()
	{

	}

	void CRuntime::OnRender(IRenderer* renderer)
	{
		renderer->Render(&mesh);
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
