#include "CRuntime.hpp"

#include <BetterThanNothing.hpp>
#include "Renderer/Resources/CModelLoader_interim.hpp"

#define ASSETS_PATH "../../Assets/Models/"

#define TEXTURE_PATH ASSETS_PATH"viking_room/viking_room.png"
#define MODEL_PATH ASSETS_PATH"viking_room/viking_room.obj"

namespace BetterThanNothing
{
	ModelData model;
	glm::mat4 modelTransform;

	void CRuntime::OnEnable(IRenderer* renderer)
	{
		renderer->AddPipeline([&](IPipelineBuilder* builder) {
			builder->SetName("main");
			builder->AddVertexShader("main/main.vert");
			builder->AddFragmentShader("main/main.frag");
		});

		model = CModelLoader_interim::LoadModel(MODEL_PATH);
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
		renderList->Submit(&model, TEXTURE_PATH, modelTransform);
	}

	void CRuntime::OnDisable()
	{

	}
};

BetterThanNothing::CApplication* CreateApplication()
{
	return new BetterThanNothing::CRuntime();
}
