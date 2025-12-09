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

		mesh.AddVertex({-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f});
		mesh.AddVertex({0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f});
		mesh.AddVertex({0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f});
		mesh.AddVertex({-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f});

		mesh.AddVertex({-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f});
		mesh.AddVertex({0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f});
		mesh.AddVertex({0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f});
		mesh.AddVertex({-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f});

		mesh.AddIndices({
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		});

		modelTransform = glm::mat4(1.0f);
	}

	void CRuntime::OnUpdate()
	{
		static glm::vec2 lastMousePos = CInputManager::GetMousePosition();
		const glm::vec2 currentMousePos = CInputManager::GetMousePosition();

		static float accumulatedAngle = 0.0f; // radians

		if (CInputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			constexpr float sensibility = 0.01f;  // radians per frame

			const glm::vec2 delta = currentMousePos - lastMousePos;
			const float deltaAngle = delta.x * sensibility; // radians

			accumulatedAngle += deltaAngle;
		}

		modelTransform = glm::rotate(glm::mat4(1.0f), accumulatedAngle, glm::vec3(0.0f, 0.0f, 1.0f));

		lastMousePos = currentMousePos;
	}

	void CRuntime::OnRender(CRenderList* renderList)
	{
		renderList->Render(&mesh, modelTransform, "../../Assets/Textures/texture.jpg");
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
