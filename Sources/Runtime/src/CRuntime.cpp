#include "CRuntime.hpp"

#include <Module_Engine/CModelLoader_interim.hpp>
#include <Module_Engine/CRenderContext.hpp>
#include <Module_Engine/CUpdateContext.hpp>

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>

#include <Module_Data/Graphics/ModelData.hpp>

namespace BetterThanNothing
{
	ModelData sponzaModel;
	glm::mat4 sponzaTransform;

	ModelData robotModel;
	glm::mat4 robotTransform;

	void CRuntime::OnEnable()
	{
		sponzaModel = CModelLoader_interim::LoadModel("../../../Assets/Models/sponza/sponza.obj");
		sponzaTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));

		robotModel = CModelLoader_interim::LoadModel("../../../Assets/Models/robot/robot.obj");
		robotTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.5f, 0.0f));
	}

	void CRuntime::OnUpdate(const CUpdateContext* ctx)
	{
		if (ctx->IsMouseButtonPressed(EMouseButton::Left))
		{
			constexpr float rotateSpeed = 0.1f;
			const glm::vec2 delta = ctx->GetMouseDelta();
			ctx->RotateCamera(-delta.x * rotateSpeed, -delta.y * rotateSpeed);
		}

		const float deltaTime = ctx->GetDeltaTime();
		const float moveSpeed = 5.0f * deltaTime;
		constexpr auto up = glm::vec3(0.0f, 1.0f, 0.0f);

		auto deltaPos = glm::vec3(0.0f);

		if (ctx->IsKeyPressed(EKey::W))			deltaPos += ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::S))			deltaPos -= ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::D))			deltaPos += ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::A))			deltaPos -= ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::Space))		deltaPos += up * moveSpeed;
		if (ctx->IsKeyPressed(EKey::LeftShift))	deltaPos -= up * moveSpeed;

		ctx->MoveCamera(deltaPos);

		robotTransform = glm::rotate(robotTransform, glm::radians(20.0f * deltaTime), up);
	}

	void CRuntime::OnRender(const CRenderContext* ctx)
	{
		ctx->Submit(&sponzaModel, sponzaTransform);
		ctx->Submit(&robotModel, robotTransform, "../../../Assets/Models/robot/robot.png");
	}

	void CRuntime::OnDisable()
	{

	}
};

int main()
{
	BetterThanNothing::CRuntime runtime;
	runtime.Run();

	return 0;
}