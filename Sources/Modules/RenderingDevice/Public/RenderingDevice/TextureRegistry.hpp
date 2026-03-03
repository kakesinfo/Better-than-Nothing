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

#include "Handle.hpp"
#include "HandleRegistry.hpp"
#include "TextureDescription.hpp"

namespace BetterThanNothing
{
	namespace RenderingApi
	{
		class Device;
		class CommandPool;
		class Queue;
		class Allocator;
		struct Texture;
	}

	struct TextureRegistryCreateInfo
	{
		RenderingApi::Device*	   device;
		RenderingApi::CommandPool* commandPool;
		RenderingApi::Queue*	   graphicsQueue;
		RenderingApi::Allocator*   allocator;
	};

	class TextureRegistry
	{
	public:
		explicit TextureRegistry(const TextureRegistryCreateInfo& createInfo);
		~TextureRegistry();

		static std::unique_ptr<TextureRegistry> Create(const TextureRegistryCreateInfo& createInfo);

	private:
		bool Internal_CreateTextureImage(const TextureImageDescription& desc);
		bool Internal_CreateTextureImageView(const TextureImageViewDescription& desc);
		bool Internal_CopyDataToTextureImage(const CopyDataToTextureImageDescription& desc);
		bool Internal_CreateTextureMipmaps(const Handle<RenderingApi::Texture>& destination);

	public:
		Handle<RenderingApi::Texture> CreateTextureFromData(const TextureFromDataDescription& desc);
		void						  DestroyTexture(const Handle<RenderingApi::Texture>& handle);
		RenderingApi::Texture*		  GetTexture(const Handle<RenderingApi::Texture>& handle);

	private:
		TextureRegistryCreateInfo			  m_createInfo;
		HandleRegistry<RenderingApi::Texture> m_textures;
	};
}
