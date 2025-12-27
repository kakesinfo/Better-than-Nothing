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
#include <Module_Engine/EntryPoint.hpp>
#include <Module_Engine/ModelLoader.hpp>
#include <Module_Engine/RenderContext.hpp>
#include <Module_Engine/UpdateContext.hpp>
#include <Module_Math/Matrix4.hpp>
#include <Module_Math/Vector2.hpp>
#include <Module_Math/Vector3.hpp>

namespace BetterThanNothing
{
	void Runtime::OnEnable()
	{
		m_robotTransform = Matrix4::Identity.Translate(2.0f, 0.0f, 0.0f);
		m_robotFuture = std::async(std::launch::async,
								   []() { return ModelLoader::Load("../../../Assets/Models/robot/robot.obj"); });
	}

	void Runtime::OnUpdate(const UpdateContext* ctx)
	{
		if (ctx->IsMouseButtonPressed(EMouseButton::Left))
		{
			constexpr float rotateSpeed = 0.1f;
			const Vector2	delta = ctx->GetMouseDelta();
			ctx->RotateCamera(-delta.GetX() * rotateSpeed, -delta.GetY() * rotateSpeed);
		}

		const float deltaTime = ctx->GetDeltaTime();
		const float moveSpeed = 5.0f * deltaTime;

		auto deltaPos = Vector3::Zero;

		if (ctx->IsKeyPressed(EKey::W))
			deltaPos += ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::S))
			deltaPos -= ctx->GetCameraForward() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::D))
			deltaPos += ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::A))
			deltaPos -= ctx->GetCameraRight() * moveSpeed;
		if (ctx->IsKeyPressed(EKey::Space))
			deltaPos += Vector3::Up * moveSpeed;
		if (ctx->IsKeyPressed(EKey::LeftShift))
			deltaPos -= Vector3::Up * moveSpeed;

		ctx->MoveCamera(deltaPos);

		m_robotTransform = m_robotTransform.Rotate(20.0f * deltaTime, Vector3::Up);

		SyncModels();
	}

	void Runtime::OnRender(const RenderContext* ctx)
	{
		if (m_robotModel.has_value())
			ctx->Submit(&m_robotModel.value(), m_robotTransform, "../../../Assets/Models/robot/robot.png");
	}

	void Runtime::OnDisable() {}

	void Runtime::SyncModels()
	{
		if (m_robotFuture.valid() && m_robotFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			m_robotModel = m_robotFuture.get();
	}

	std::unique_ptr<IEngineApplication> CreateEngineApplication()
	{
		return std::make_unique<Runtime>();
	}
};
