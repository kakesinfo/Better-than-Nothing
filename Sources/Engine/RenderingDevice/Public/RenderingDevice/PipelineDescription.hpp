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
#include "Types.hpp"
#include "VertexLayout.hpp"
#include <string>
#include <vector>

namespace BetterThanNothing
{
	class Handle;

	struct PipelineDescription
	{
		std::vector<Handle> shaders;
		std::string			layoutName;
		VertexLayout		vertexLayout;
		TextureFormat			   colorFormat = TextureFormat::Undefined;
		std::vector<TextureFormat> colorFormats;
		TextureFormat			   depthFormat = TextureFormat::Undefined;
		bool				useMsaa = true;
		bool				wireframe = false;
		bool				depthTest = true;
		bool				depthWrite = true;
		bool				blendEnable = true;
		bool				alphaToCoverage = true;
		CullMode			cullMode = CullMode::Back;
	};
}
