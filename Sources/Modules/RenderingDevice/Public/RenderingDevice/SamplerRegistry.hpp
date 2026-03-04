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
#include "HandleRegistry.hpp"
#include "SamplerDescription.hpp"
#include <unordered_map>

namespace BetterThanNothing
{
	namespace RenderingApi
	{
		class Device;
		struct Sampler;
	}

	class SamplerRegistry
	{
	public:
		explicit SamplerRegistry(RenderingApi::Device* device);
		~SamplerRegistry();

		static std::unique_ptr<SamplerRegistry> Create(RenderingApi::Device* device);

		Handle GetOrCreateSampler(const SamplerDescription& desc);
		void						  DestroySampler(Handle handle);
		RenderingApi::Sampler*		  GetSampler(Handle handle);


	private:
		RenderingApi::Device*				  m_device;
		HandleRegistry<RenderingApi::Sampler, HandleType::Sampler> m_samplers;
		std::unordered_map<SamplerDescription, Handle, SamplerDescriptionHash, SamplerDescriptionEqual> m_samplersLUT;
	};
}
