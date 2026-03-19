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

#include <RenderingApi/Core/Instance.hpp>

namespace BetterThanNothing
{
	struct RenderingDeviceInternals
	{
		std::unique_ptr<RenderingApi::Instance>		  instance;
		std::unique_ptr<RenderingApi::Surface>		  surface;
		std::unique_ptr<RenderingApi::Device>		  device;
		std::unique_ptr<RenderingApi::Queue>		  graphicsQueue;
		std::unique_ptr<RenderingApi::Queue>		  presentQueue;
		std::unique_ptr<RenderingApi::Allocator>	  allocator;
		std::unique_ptr<RenderingApi::CommandPool>	  commandPool;
		std::unique_ptr<RenderingApi::SwapChain>	  swapChain;
		std::unique_ptr<RenderingApi::DescriptorPool> descriptorPool;
	};
}
