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

#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice_Internals.hpp"

#include "RenderingApi/Resources/Sampler.hpp"
#include "RenderingDevice/Handle.hpp"
#include <Module_Platform/Manager/WindowManager.hpp>
#include <RenderingApi.hpp>
#include <cmath>

namespace BetterThanNothing
{
	static VkFilter Internal_RetrieveSamplerFilter(const SamplerFilter& filter)
	{
		switch (filter)
		{
			case SamplerFilter::Linear:
				return VK_FILTER_LINEAR;
			case SamplerFilter::Nearest:
				return VK_FILTER_NEAREST;
		}
		throw std::runtime_error("Unknown sampler filter");
	}

	static VkSamplerAddressMode Internal_RetrieveSamplerAddressMode(const SamplerAddressMode& addressMode)
	{
		switch (addressMode)
		{
			case SamplerAddressMode::ClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case SamplerAddressMode::ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case SamplerAddressMode::MirrorRepeat:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case SamplerAddressMode::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
		throw std::runtime_error("Unknown sampler adress mode");
	}

	static VkBorderColor Internal_RetrieveBolderColor(const BorderColor& borderColor)
	{
		switch (borderColor)
		{
			case BorderColor::FloatOpaqueBlack:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case BorderColor::FloatOpaqueWhite:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case BorderColor::FloatTransparentBlack:
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColor::IntOpaqueBlack:
				return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case BorderColor::IntOpaqueWhite:
				return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			case BorderColor::IntTransparentBlack:
				return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		}
		throw std::runtime_error("Unknown border color");
	}

	static VkCompareOp Internal_RetrieveCompareOp(const CompareOp& compareOp)
	{
		switch (compareOp)
		{
			case CompareOp::Always:
				return VK_COMPARE_OP_ALWAYS;
			case CompareOp::Equal:
				return VK_COMPARE_OP_EQUAL;
			case CompareOp::Greater:
				return VK_COMPARE_OP_GREATER;
			case CompareOp::GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOp::Less:
				return VK_COMPARE_OP_LESS;
			case CompareOp::LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOp::Never:
				return VK_COMPARE_OP_NEVER;
			case CompareOp::NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
		}
		throw std::runtime_error("Unknown compare op");
	}

	static VkSamplerMipmapMode Internal_RetrieveSamplerMipmapMode(const SamplerMipmapMode& mipmapMode)
	{
		switch (mipmapMode)
		{
			case SamplerMipmapMode::Linear:
				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			case SamplerMipmapMode::Nearest:
				return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
		throw std::runtime_error("Unknown sampler mipmap mode");
	}

	Handle RenderingDevice::GetOrCreateTextureSampler(const SamplerDescription& desc)
	{
		if (m_samplersLUT.contains(desc))
			return m_samplersLUT[desc];

		const Handle		   handle = m_self->samplers.Allocate();
		RenderingApi::Sampler* newSampler = m_self->samplers.Get(handle);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = Internal_RetrieveSamplerFilter(desc.magFilter);
		samplerInfo.minFilter = Internal_RetrieveSamplerFilter(desc.minFilter);
		samplerInfo.addressModeU = Internal_RetrieveSamplerAddressMode(desc.addressModeU);
		samplerInfo.addressModeV = Internal_RetrieveSamplerAddressMode(desc.addressModeV);
		samplerInfo.addressModeW = Internal_RetrieveSamplerAddressMode(desc.addressModeW);
		samplerInfo.anisotropyEnable = desc.anisotropyEnable;
		samplerInfo.maxAnisotropy = desc.maxAnisotropy == -1 ? m_self->device->GetMaxAnisotropy() : desc.maxAnisotropy;
		samplerInfo.borderColor = Internal_RetrieveBolderColor(desc.borderColor);
		samplerInfo.compareEnable = desc.compareEnable;
		samplerInfo.compareOp = Internal_RetrieveCompareOp(desc.compareOp);
		samplerInfo.mipmapMode = Internal_RetrieveSamplerMipmapMode(desc.mipmapMode);
		samplerInfo.minLod = desc.minLod;
		samplerInfo.maxLod = desc.maxLod;
		samplerInfo.mipLodBias = desc.mipLodBias;

		if (m_self->device->CreateSampler(samplerInfo, &newSampler->value) != VK_SUCCESS)
			throw std::runtime_error("failed to create vulkan sampler");

		m_samplersLUT[desc] = handle;

		return handle;
	}

	void RenderingDevice::DestroyTextureSampler(const Handle& handle)
	{
		if (!m_self->samplers.Has(handle))
			throw std::runtime_error("Invalid sampler handle!");

		const RenderingApi::Sampler* sampler = m_self->samplers.Get(handle);
		m_self->device->DestroySampler(sampler->value);
		m_self->samplers.Destroy(handle);
	}

	RenderingApi::Sampler* RenderingDevice::GetSampler(const Handle& handle)
	{
		if (!m_self->samplers.Has(handle))
			throw std::runtime_error("Invalid sampler handle!");

		return m_self->samplers.Get(handle);
	}
}
