#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>

namespace BetterThanNothing
{
	MeshData mesh;
	glm::mat4 modelTransform;

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
		static auto startTime = std::chrono::high_resolution_clock::now();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float>(currentTime - startTime).count();

		modelTransform = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void CRuntime::OnRender(CRenderList* renderList)
	{
		renderList->Render(&mesh, modelTransform);
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
