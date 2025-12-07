#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	MeshData mesh;

	void CRuntime::OnEnable(IRenderer* renderer)
	{
		renderer->AddPipeline([&](IPipelineBuilder* builder) {
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		});

		mesh.AddVertex({-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f});
		mesh.AddVertex({0.5f, -0.5f}, {0.0f, 1.0f, 0.0f});
		mesh.AddVertex({0.5f, 0.5f}, {0.0f, 0.0f, 1.0f});
		mesh.AddVertex({-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f});
		mesh.AddIndices({0, 1, 2, 2, 3, 0});
	}

	void CRuntime::OnUpdate()
	{

	}

	void CRuntime::OnRender(CRenderQueue* renderQueue)
	{
		renderQueue->Render(&mesh, glm::mat4(1.0f));
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
