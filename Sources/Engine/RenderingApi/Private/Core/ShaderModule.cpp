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

#include "RenderingApi.hpp"

namespace BetterThanNothing::RenderingApi
{
	ShaderModule::ShaderModule(Device* device, const VkShaderModuleCreateInfo* createInfo)
		: m_device(device)
	{
		if (vkCreateShaderModule(m_device->Handle(), createInfo, nullptr, &m_shaderModule) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module!");
	}

	ShaderModule::~ShaderModule()
	{
		if (m_device && m_shaderModule)
			vkDestroyShaderModule(m_device->Handle(), m_shaderModule, nullptr);
	}

	VkShaderModule ShaderModule::Handle() const
	{
		return m_shaderModule;
	}
}
