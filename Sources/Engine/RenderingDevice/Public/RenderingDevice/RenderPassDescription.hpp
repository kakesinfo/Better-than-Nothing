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

#include "Handle.hpp"
#include "Types.hpp"
#include <Common/Containers/FixedArray.hpp>
#include <array>
#include <cstdint>
#include <optional>

namespace BetterThanNothing
{
	struct ColorAttachmentDesc
	{
		Handle				 target{};
		bool				 useSwapChainImage = false;
		uint32_t			 swapChainImageIndex = 0;

		Handle				 resolveTarget{};
		bool				 resolveToSwapChain = false;

		LoadOp				 loadOp = LoadOp::Clear;
		StoreOp				 storeOp = StoreOp::Store;
		std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	struct DepthAttachmentDesc
	{
		Handle	 target{};
		LoadOp	 loadOp = LoadOp::Clear;
		StoreOp	 storeOp = StoreOp::DontCare;
		float	 clearDepth = 1.0f;
		uint32_t clearStencil = 0;
	};

	struct RenderPassDescriptor
	{
		static constexpr std::size_t MaxColorAttachments = 8;

		FixedArray<ColorAttachmentDesc, MaxColorAttachments> colorAttachments;
		std::optional<DepthAttachmentDesc>					 depthAttachment;
		uint32_t											 width = 0;
		uint32_t											 height = 0;
	};
}
