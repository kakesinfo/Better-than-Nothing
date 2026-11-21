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

#include "Common/Logger.hpp"
#include "RenderingDevice/Handle.hpp"
#include "RenderingDevice/RenderingDevice.hpp"
#include "RenderingDevice/ShaderDescription.hpp"
#include "RenderingDevice_Internals.hpp"

#include <RenderingApi.hpp>

namespace BetterThanNothing
{
	static bool Internal_CreateShader(RenderingApi::Shader* dstShader, const ShaderDescription& desc, const RenderingApi::Device* device)
	{
		if (desc.spirv.empty())
			return false;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = desc.spirv.size() * sizeof(uint32_t);
		createInfo.pCode = desc.spirv.data();

		if (device->CreateShaderModule(createInfo, &dstShader->module) != VK_SUCCESS)
			return false;

		dstShader->type = desc.type;
		dstShader->name = desc.name;

		return true;
	}

	Handle RenderingDevice::CreateShader(const ShaderDescription& desc)
	{
		const auto handle = m_self->shaders.Allocate();

		if (!Internal_CreateShader(m_self->shaders.Get(handle), desc, m_self->device.get()))
		{
			m_self->shaders.Destroy(handle);
			throw std::runtime_error("Failed to create shader: " + desc.name);
		}

		return handle;
	}

	void RenderingDevice::DestroyShader(const Handle& handle)
	{
		if (!m_self->shaders.Has(handle))
			throw std::runtime_error("Invalid shader handle!");

		const RenderingApi::Shader* shader = m_self->shaders.Get(handle);
		m_self->device->DestroyShaderModule(shader->module);
		m_self->shaders.Destroy(handle);
	}
}
