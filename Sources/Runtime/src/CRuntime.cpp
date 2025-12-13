#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>

#include "Mesh/CPlaneMesh.hpp"
#include "Renderer/Resources/CModelLoader_interim.hpp"

namespace BetterThanNothing
{
	ModelData sponzaModel;
	glm::mat4 sponzaTransform;

	ModelData robotModel;
	glm::mat4 robotTransform;

	void CRuntime::OnEnable(IRenderer* renderer)
	{
		renderer->AddPipeline([&](IPipelineBuilder* builder) {
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		});

		sponzaModel = CModelLoader_interim::LoadModel("../../Assets/Models/sponza/sponza.obj");
		sponzaTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));

		robotModel = CModelLoader_interim::LoadModel("../../Assets/Models/robot/robot.obj");
		robotTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.5f, 0.0f));
	}

	void CRuntime::OnUpdate(CCamera* camera, const float deltaTime)
	{
		if (CInputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			constexpr float rotateSpeed = 0.1f;
			const glm::vec2 delta = CInputManager::GetMouseDelta();
			camera->Rotate(-delta.x * rotateSpeed, -delta.y * rotateSpeed);
		}

		const float moveSpeed = 5.0f * deltaTime;

		auto deltaPos = glm::vec3(0.0f);
		if (CInputManager::IsKeyPressed(GLFW_KEY_W))
			deltaPos += camera->GetForward() * moveSpeed;
		if (CInputManager::IsKeyPressed(GLFW_KEY_S))
			deltaPos -= camera->GetForward() * moveSpeed;
		if (CInputManager::IsKeyPressed(GLFW_KEY_D))
			deltaPos += camera->GetRight() * moveSpeed;
		if (CInputManager::IsKeyPressed(GLFW_KEY_A))
			deltaPos -= camera->GetRight() * moveSpeed;
		if (CInputManager::IsKeyPressed(GLFW_KEY_SPACE))
			deltaPos += glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;
		if (CInputManager::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
			deltaPos -= glm::vec3(0.0f, 1.0f, 0.0f) * moveSpeed;

		camera->Move(deltaPos);

		robotTransform = glm::rotate(robotTransform, glm::radians(20.0f * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void CRuntime::OnRender(CRenderList* renderList)
	{
		renderList->Submit(&sponzaModel, sponzaTransform);
		renderList->Submit(&robotModel, robotTransform, "../../Assets/Models/robot/robot.png");
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
