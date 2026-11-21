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
		PipelineBlueprint* SetColorFormat(TextureFormat format);
		PipelineBlueprint* SetColorFormats(std::vector<TextureFormat> formats);
		PipelineBlueprint* SetDepthFormat(TextureFormat format);
		PipelineBlueprint* SetHasDepthAttachment(bool enable);
		PipelineBlueprint* SetUseMsaa(bool enable);
		PipelineBlueprint* SetDepthTest(bool enable);
		PipelineBlueprint* SetDepthWrite(bool enable);
		PipelineBlueprint* SetBlendEnable(bool enable);
		PipelineBlueprint* SetAlphaToCoverage(bool enable);
		PipelineBlueprint* SetWireframe(bool enable);
		PipelineBlueprint* SetCullMode(CullMode mode);

		[[nodiscard]] std::vector<ResourceId>	 GetShaders() const;
		[[nodiscard]] std::string				 GetPipelineLayout() const;
		[[nodiscard]] VertexLayout				 GetVertexLayout() const;
		[[nodiscard]] TextureFormat				 GetColorFormat() const { return m_colorFormat; }
		[[nodiscard]] std::vector<TextureFormat> GetColorFormats() const { return m_colorFormats; }
		[[nodiscard]] TextureFormat				 GetDepthFormat() const { return m_depthFormat; }
		[[nodiscard]] bool						 GetHasDepthAttachment() const { return m_hasDepthAttachment; }
		[[nodiscard]] bool						 GetUseMsaa() const { return m_useMsaa; }
		[[nodiscard]] bool						 GetDepthTest() const { return m_depthTest; }
		[[nodiscard]] bool						 GetDepthWrite() const { return m_depthWrite; }
		[[nodiscard]] bool						 GetBlendEnable() const { return m_blendEnable; }
		[[nodiscard]] bool						 GetAlphaToCoverage() const { return m_alphaToCoverage; }
		[[nodiscard]] bool						 GetWireframe() const { return m_wireframe; }
		[[nodiscard]] CullMode					 GetCullMode() const { return m_cullMode; }

	private:
		std::vector<ResourceId>	   m_shaders;
		std::string				   m_pipelineLayout;
		VertexLayout			   m_vertexLayout = {};
		TextureFormat			   m_colorFormat = TextureFormat::Undefined;
		std::vector<TextureFormat> m_colorFormats;
		TextureFormat			   m_depthFormat = TextureFormat::Undefined;
		bool					   m_hasDepthAttachment = true;
		bool					   m_useMsaa = false;
		bool					   m_depthTest = true;
		bool					   m_depthWrite = true;
		bool					   m_blendEnable = false;
		bool					   m_alphaToCoverage = false;
		bool					   m_wireframe = false;
		CullMode				   m_cullMode = CullMode::Back;
	};
}
