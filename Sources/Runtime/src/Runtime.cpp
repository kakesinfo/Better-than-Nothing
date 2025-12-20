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

#include "Module_Engine/MeshFactory.hpp"

#include <Module_Base/EKey.hpp>
#include <Module_Base/EMouseButton.hpp>
#include <Module_Data/Graphics/ModelData.hpp>
#include <Module_Engine/EntryPoint.hpp>
#include <Module_Engine/ModelLoader.hpp>
#include <Module_Engine/RenderContext.hpp>
#include <Module_Engine/UpdateContext.hpp>

namespace BetterThanNothing
{
	void Runtime::OnEnable()
	{
		m_robotTransform = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
		m_robotFuture = std::async(std::launch::async, []() {
			return ModelLoader::Load("../../../Assets/Models/robot/robot.obj");
		});

		m_sponzaTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
		m_sponzaFuture = std::async(std::launch::async, []() {
			return ModelLoader::Load("../../../Assets/Models/sponza/sponza.obj");
		});
	}

	void Runtime::OnUpdate(const UpdateContext* ctx)
	{
		if (ctx->IsMouseButtonPressed(EMouseButton::Left))
		{
			constexpr float rotateSpeed = 0.1f;
			const glm::vec2 delta = ctx->GetMouseDelta();
			ctx->RotateCamera(-delta.x * rotateSpeed, -delta.y * rotateSpeed);
		}

		const float	   deltaTime = ctx->GetDeltaTime();
		const float	   moveSpeed = 5.0f * deltaTime;
		constexpr auto up = glm::vec3(0.0f, 1.0f, 0.0f);

		auto deltaPos = glm::vec3(0.0f);

		if (ctx->IsKeyPressed(EKey::W))
			deltaPos += ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::S))
			deltaPos -= ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::D))
			deltaPos += ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::A))
			deltaPos -= ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::Space))
			deltaPos += up * moveSpeed;
		if (ctx->IsKeyPressed(EKey::LeftShift))
			deltaPos -= up * moveSpeed;

		ctx->MoveCamera(deltaPos);

		m_robotTransform = glm::rotate(m_robotTransform, glm::radians(20.0f * deltaTime), up);

		SyncModels();
	}

	void Runtime::OnRender(const RenderContext* ctx)
	{
		if (m_sponzaModel.has_value())
			ctx->Submit(&m_sponzaModel.value(), m_sponzaTransform);

		if (m_robotModel.has_value())
			ctx->Submit(&m_robotModel.value(), m_robotTransform, "../../../Assets/Models/robot/robot.png");
	}

	void Runtime::OnDisable() {}

	void Runtime::SyncModels()
	{
		if (m_sponzaFuture.valid() && m_sponzaFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			m_sponzaModel = m_sponzaFuture.get();

		if (m_robotFuture.valid() && m_robotFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			m_robotModel = m_robotFuture.get();
	}

	std::unique_ptr<IEngineApplication> CreateEngineApplication()
	{
		return std::make_unique<Runtime>();
	}
};
