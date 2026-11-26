/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "RenderPass.hpp"
#include "RenderingDevice/Handle.hpp"
#include <Data/Graphics/PointLightData.hpp>
#include <Math/Matrix4.hpp>
#include <memory>
#include <vector>

namespace BetterThanNothing
{
	class ResourceManager;
	class RenderingDevice;
	class Scene;
	class Editor;
	class DrawStream;
	struct GlobalUniformData;

	class Renderer final
	{
	public:
		explicit Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		static std::unique_ptr<Renderer> Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor);

		void AddPass(std::unique_ptr<IRenderPass> pass);
		void RenderFrame(Scene* scene);

	private:
		void CreateGlobalUniformBuffers();
		void CreateGlobalUniformDescriptorSets();
		auto GetGlobalUniformBuffer(uint32_t frameIndex) const -> GlobalUniformData*;

		void CreatePointLightBuffers();
		void CreatePointLightDescriptorSets();

		void EnsureRenderTargets();
		void DestroyRenderTargets();

		void	 BuildDrawStreams(Scene* scene, uint32_t frameIndex);
		uint32_t UploadPointLights(const Scene* scene, uint32_t frameIndex) const;

		RenderingDevice*			m_renderingDevice = nullptr;
		ResourceManager*			m_resourceManager = nullptr;
		Editor*						m_editor = nullptr;
		std::unique_ptr<DrawStream> m_opaqueDrawStream;
		std::unique_ptr<DrawStream> m_transparentDrawStream;

		std::vector<std::unique_ptr<IRenderPass>> m_passes;

		Handle	 m_colorTarget{};
		Handle	 m_depthTarget{};
		Handle	 m_gbufferAlbedo{};
		Handle	 m_gbufferNormal{};
		Handle	 m_gbufferMaterial{};
		Handle	 m_gbufferMotion{};
		Handle	 m_prevDepthTarget{};
		Handle	 m_prevNormalTarget{};
		Handle	 m_hdrColor{};
		uint64_t m_lastSwapChainGeneration = 0;
		Matrix4	 m_prevUnjitteredViewProj = Matrix4::Identity;
		uint32_t m_frameIndex = 0;

		std::vector<Handle>					m_globalUniformBuffer;
		std::vector<Handle>					m_globalDescriptorSets;
		std::vector<Handle>					m_pointLightBuffer;
		std::vector<Handle>					m_pointLightDescriptorSets;
		mutable std::vector<PointLightData> m_pointLightsScratch;
		mutable bool						m_warnedLightLimit = false;
	};
}
