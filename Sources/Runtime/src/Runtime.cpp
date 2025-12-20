/*
*	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Runtime.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Engine/ModelLoader.hpp>
#include <Module_Engine/RenderContext.hpp>
#include <Module_Engine/UpdateContext.hpp>
#include <Module_Engine/EntryPoint.hpp>

namespace BetterThanNothing
{
	void Runtime::OnEnable()
	{
		m_sponzaModel = ModelLoader::Load("../../../Assets/Models/sponza/sponza.obj");
		m_sponzaTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));

		m_robotModel = ModelLoader::Load("../../../Assets/Models/robot/robot.obj");
		m_robotTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.5f, 0.0f));
	}

	void Runtime::OnUpdate(const UpdateContext* ctx)
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

		m_robotTransform = glm::rotate(m_robotTransform, glm::radians(20.0f * deltaTime), up);
	}

	void Runtime::OnRender(const RenderContext* ctx)
	{
		ctx->Submit(&m_sponzaModel, m_sponzaTransform);
		ctx->Submit(&m_robotModel, m_robotTransform, "../../../Assets/Models/robot/robot.png");
	}

	void Runtime::OnDisable()
	{

	}

	std::unique_ptr<IEngineApplication> CreateEngineApplication()
	{
		return std::make_unique<Runtime>();
	}
};
