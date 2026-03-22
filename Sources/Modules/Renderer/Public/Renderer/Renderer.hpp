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

#include "RenderingDevice/Handle.hpp"

namespace BetterThanNothing
{
	struct GlobalUniformData;
	class Window;

	namespace RenderingApi
	{
		class Pipeline;
		class PipelineBuilder;
		class Semaphore;
	}

	class RenderCache;
	class Camera;
	struct CameraData;
	class DrawStream;
	struct DirectionalLightData;
	class ImGuiPass;
	struct DrawStreamView;
	class ResourceManager;
	class RenderingDevice;
	class Scene;
	class Editor;

	class Renderer final
	{
	public:
		explicit Renderer(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* editor);
		~Renderer();

		static std::unique_ptr<Renderer> Create(RenderingDevice* renderingDevice, ResourceManager* resourceManager, Editor* debugUI);

		void RenderFrame(Scene* scene) const;

	private:
		auto CreateGlobalUniformBuffers() -> void;
		auto CreateGlobalUniformDescriptorSets() -> void;
		auto GetGlobalUniformBuffer(uint32_t frameIndex) const -> GlobalUniformData*;

		void CreatePointLightBuffers();
		void CreatePointLightDescriptorSets();

		void RecordCommandBuffer(Scene* scene, uint32_t imageIndex) const;
		void DrawPointLights(const Scene* scene, uint32_t frameIndex) const;

		void ExecuteForwardPass(uint32_t imageIndex, const DrawStreamView& drawStream) const;
		void ExecuteEditorPass(Scene* scene, uint32_t imageIndex) const;

		static VkVertexInputBindingDescription				  GetVertexBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions();

	public:
		void AddPipeline(const std::function<void(RenderingApi::PipelineBuilder*)>& callback);

	private:
		RenderingDevice*			m_renderingDevice = nullptr;
		ResourceManager*			m_resourceManager = nullptr;
		Editor*						m_editor = nullptr;
		std::unique_ptr<DrawStream> m_drawStream;

		std::unique_ptr<RenderingApi::Pipeline> m_defaultPipeline;
		std::vector<Handle>						m_globalUniformBuffer;
		std::vector<Handle>						m_globalDescriptorSets;
		std::vector<Handle>						m_pointLightBuffer;
		std::vector<Handle>						m_pointLightDescriptorSets;
	};
}
