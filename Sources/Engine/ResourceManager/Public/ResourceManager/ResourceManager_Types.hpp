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

#include <RenderingDevice/Handle.hpp>
#include <memory>

namespace BetterThanNothing
{
	struct MeshResourceCache
	{
		Handle	 vertexBuffer;
		uint32_t vertexCount = 0;
		Handle	 indexBuffer;
		uint32_t indexCount = 0;
	};

	struct TextureResourceCache
	{
		int32_t width{};
		int32_t height{};
		int32_t channels{};
		int32_t size{};
		Handle	textureHandle;
		Handle	samplerHandle;
		Handle	descriptorSetHandle;
	};

	struct ShaderResourceCache
	{
		std::string name;
		EShaderType type;
		Handle		module;
	};

	struct PipelineResourceCache
	{
		std::string name;
		std::string layoutName;
		Handle		pipeline;
	};
}
