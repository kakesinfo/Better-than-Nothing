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

#pragma once

#include "IEngineApplication.hpp"

namespace BetterThanNothing
{
	class ResourceManager;
	class RenderCache;
	class RenderList;
	class LegacyRenderer;
	class Camera;

	struct VulkanContext;

	class Engine
	{
	public:
		explicit Engine(IEngineApplication* application);
		~Engine() = default;

	private:
		void OnUpdate(float deltaTime) const;
		void OnRender() const;

	public:
		void Run() const;

	private:
		std::unique_ptr<VulkanContext>	 m_context;
		std::unique_ptr<RenderCache>	 m_renderCache;
		std::unique_ptr<ResourceManager> m_resourceManager;
		std::unique_ptr<RenderList>		 m_renderList;
		std::unique_ptr<LegacyRenderer>	 m_legacyRenderer;
		std::unique_ptr<Camera>			 m_camera;

		IEngineApplication* m_application;
	};
};
