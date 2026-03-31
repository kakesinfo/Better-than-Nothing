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

namespace BetterThanNothing
{
	class RenderingDevice;

	namespace RenderingApi
	{
		class DescriptorSetLayout;
		class PipelineLayout;
	}

	struct PipelineLayoutBlueprintPimpl;

	class PipelineLayoutBlueprint
	{
	public:
		explicit PipelineLayoutBlueprint(RenderingDevice* renderingDevice);
		~PipelineLayoutBlueprint();

		PipelineLayoutBlueprint* AddDescriptorSetLayout(const RenderingApi::DescriptorSetLayout* layout);
		PipelineLayoutBlueprint* AddDescriptorSetLayout(const std::string& layoutName);
		PipelineLayoutBlueprint* AddPushConstant(EShaderType stage, uint32_t size);

		template <typename T>
		PipelineLayoutBlueprint* AddPushConstant(const EShaderType stage)
		{
			return AddPushConstant(stage, sizeof(T));
		}

		[[nodiscard]] std::unique_ptr<RenderingApi::PipelineLayout> Build() const;

	private:
		RenderingDevice*							  m_renderingDevice;
		std::unique_ptr<PipelineLayoutBlueprintPimpl> m_self;
	};
}
