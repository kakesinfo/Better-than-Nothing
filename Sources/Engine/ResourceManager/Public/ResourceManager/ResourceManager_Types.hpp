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

#include <Math/Matrix4.hpp>
#include <Math/Vector3.hpp>
#include <Math/Vector4.hpp>
#include <RenderingDevice/Handle.hpp>
#include <memory>
#include <string>

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

	struct MaterialUniformData
	{
		alignas(16) Vector4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
		alignas(16) Vector3 emissiveFactor{0.0f, 0.0f, 0.0f};
		float metallicFactor{1.0f};
		float roughnessFactor{1.0f};
		float alphaCutoff{0.5f};
		uint32_t flags{0};
	};

	struct MaterialResource
	{
		ResourceId id{0};
		std::string name;
		ResourceId albedoTextureId{0};
		ResourceId normalTextureId{0};
		ResourceId metallicRoughnessTextureId{0};
		ResourceId occlusionTextureId{0};
		ResourceId emissiveTextureId{0};
		MaterialUniformData parameters;
	};

	struct MaterialResourceCache
	{
		ResourceId id{0};
		Handle descriptorSetHandle;
		Handle uniformBufferHandle;
		MaterialResource material;
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
