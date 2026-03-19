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

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING

#include <Editor/Editor.hpp>
#include <Module_Platform/Window.hpp>
#include <RenderingApi/Core/Device.hpp>
#include <RenderingApi/Core/Instance.hpp>
#include <RenderingApi/Core/Queue.hpp>
#include <RenderingApi/Core/SwapChain.hpp>
#include <RenderingDevice/RenderingDevice.hpp>

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace BetterThanNothing
{
	struct EditorInternals
	{
		VkDescriptorPool imguiPool = VK_NULL_HANDLE;
	};

	Editor::Editor(Window* window, RenderingDevice* renderingDevice) : m_window(window), m_renderingDevice(renderingDevice)
	{
		m_internals = std::make_unique<EditorInternals>();

		const VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000}, {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000}, {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(renderingDevice->GetDevice()->Handle(), &pool_info, nullptr, &m_internals->imguiPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create ImGui descriptor pool!");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplSDL3_InitForVulkan(window->Handle());

		static std::vector<VkFormat> formats;
		formats = {renderingDevice->GetSwapChain()->GetImageFormat()};

		VkPipelineRenderingCreateInfoKHR pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineInfo.colorAttachmentCount = static_cast<uint32_t>(formats.size());
		pipelineInfo.pColorAttachmentFormats = formats.data();
		pipelineInfo.depthAttachmentFormat = renderingDevice->GetSwapChain()->GetDepthFormat();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.ApiVersion = renderingDevice->GetInstance()->GetApiVersion();
		initInfo.Instance = renderingDevice->GetInstance()->Handle();
		initInfo.PhysicalDevice = renderingDevice->GetDevice()->PhysicalHandle();
		initInfo.Device = renderingDevice->GetDevice()->Handle();
		initInfo.Queue = renderingDevice->GetGraphicsQueue()->Handle();
		initInfo.DescriptorPool = m_internals->imguiPool;
		initInfo.MinImageCount = renderingDevice->GetSwapChain()->GetImageCount();
		initInfo.ImageCount = renderingDevice->GetSwapChain()->GetImageCount();
		// Note: Stencil format can be added if needed
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineInfo;
		initInfo.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
		initInfo.UseDynamicRendering = true;

		if (!ImGui_ImplVulkan_Init(&initInfo))
			throw std::runtime_error("failed to initialize ImGui");

		m_window->SetEventCallback([](const SDL_Event& event) -> bool {
			ImGui_ImplSDL3_ProcessEvent(&event);

			const ImGuiIO& io = ImGui::GetIO();

			const bool isMouseEvent = (event.type >= SDL_EVENT_MOUSE_MOTION && event.type <= SDL_EVENT_MOUSE_WHEEL);
			if (isMouseEvent && io.WantCaptureMouse)
				return true;

			const bool isKeyboardEvent = (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_TEXT_INPUT);
			if (isKeyboardEvent && io.WantCaptureKeyboard)
				return true;

			return false;
		});
	}

	Editor::~Editor()
	{
		m_renderingDevice->WaitIdle();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(m_renderingDevice->GetDevice()->Handle(), m_internals->imguiPool, nullptr);
	}

	std::unique_ptr<Editor> Editor::Create(Window* window, RenderingDevice* renderingDevice)
	{
		return std::make_unique<Editor>(window, renderingDevice);
	}

	void Editor::NewFrame() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}
}
