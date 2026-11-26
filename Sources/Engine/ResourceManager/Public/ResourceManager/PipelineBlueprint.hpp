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

#include <Common/ResourceId.hpp>
#include <RenderingDevice/Types.hpp>
#include <RenderingDevice/VertexLayout.hpp>
#include <string>
#include <vector>

namespace BetterThanNothing
{
	class PipelineBlueprint
	{
	public:
		explicit PipelineBlueprint() = default;
		~PipelineBlueprint() = default;

		PipelineBlueprint* AddShader(ResourceId id);
		PipelineBlueprint* SetPipelineLayout(const std::string& name);
		PipelineBlueprint* SetVertexLayout(const VertexLayout& layout);
		PipelineBlueprint* SetDepthTest(bool enable);
		PipelineBlueprint* SetDepthWrite(bool enable);
		PipelineBlueprint* SetBlendEnable(bool enable);
		PipelineBlueprint* SetAlphaToCoverage(bool enable);
		PipelineBlueprint* SetWireframe(bool enable);
		PipelineBlueprint* SetCullMode(CullMode mode);
		PipelineBlueprint* SetUseMsaa(bool enable);
		PipelineBlueprint* SetColorFormat(TextureFormat format);
		PipelineBlueprint* SetDepthFormat(TextureFormat format);
		PipelineBlueprint* SetColorFormats(std::vector<TextureFormat> formats);

		[[nodiscard]] std::vector<ResourceId>		  GetShaders() const;
		[[nodiscard]] std::string					  GetPipelineLayout() const;
		[[nodiscard]] VertexLayout					  GetVertexLayout() const;
		[[nodiscard]] bool							  GetDepthTest() const { return m_depthTest; }
		[[nodiscard]] bool							  GetDepthWrite() const { return m_depthWrite; }
		[[nodiscard]] bool							  GetBlendEnable() const { return m_blendEnable; }
		[[nodiscard]] bool							  GetAlphaToCoverage() const { return m_alphaToCoverage; }
		[[nodiscard]] bool							  GetWireframe() const { return m_wireframe; }
		[[nodiscard]] CullMode						  GetCullMode() const { return m_cullMode; }
		[[nodiscard]] bool							  GetUseMsaa() const { return m_useMsaa; }
		[[nodiscard]] TextureFormat					  GetColorFormat() const { return m_colorFormat; }
		[[nodiscard]] TextureFormat					  GetDepthFormat() const { return m_depthFormat; }
		[[nodiscard]] const std::vector<TextureFormat>& GetColorFormats() const { return m_colorFormats; }

	private:
		std::vector<ResourceId>	   m_shaders;
		std::string				   m_pipelineLayout;
		VertexLayout			   m_vertexLayout = {};
		bool					   m_depthTest = true;
		bool					   m_depthWrite = true;
		bool					   m_blendEnable = true;
		bool					   m_alphaToCoverage = true;
		bool					   m_wireframe = false;
		CullMode				   m_cullMode = CullMode::Back;
		bool					   m_useMsaa = true;
		TextureFormat			   m_colorFormat = TextureFormat::Undefined;
		TextureFormat			   m_depthFormat = TextureFormat::Undefined;
		std::vector<TextureFormat> m_colorFormats;
	};
}
