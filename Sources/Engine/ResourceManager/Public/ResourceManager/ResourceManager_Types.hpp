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

#include <Common/EShaderType.hpp>
#include <Common/ResourceId.hpp>
#include <Math/Vector3.hpp>
#include <RenderingDevice/Handle.hpp>
#include <cstdint>
#include <string>

namespace BetterThanNothing
{
	enum class EAlphaMode : uint8_t
	{
		Opaque = 0,
		Mask,
		Blend
	};

	struct MeshResourceCache
	{
		Handle	 vertexBuffer;
		uint32_t vertexCount = 0;
		Handle	 indexBuffer;
		uint32_t indexCount = 0;
		Vector3	 boundsMin = Vector3(-0.5f);
		Vector3	 boundsMax = Vector3(0.5f);
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

	struct MaterialResourceCache
	{
		std::string name;
		EAlphaMode	alphaMode = EAlphaMode::Opaque;
		float		alphaCutoff = 0.5f;
		ResourceId	diffuseTextureId = InvalidResourceId;
		ResourceId	normalTextureId = InvalidResourceId;
		std::string pipelineName = "Forward::Opaque";
		Handle		descriptorSetHandle;
	};

	struct ShaderResourceCache
	{
		std::string name;
		EShaderType type = EShaderType::Vertex;
		Handle		module;
	};

	struct PipelineResourceCache
	{
		std::string name;
		std::string layoutName;
		Handle		pipeline;
	};
}
