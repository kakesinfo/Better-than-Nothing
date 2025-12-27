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

namespace BetterThanNothing
{
	struct DrawCommand
	{
		VkPipeline		 pipeline;
		VkPipelineLayout pipelineLayout;

		VkBuffer vertexBuffer;
		VkBuffer indexBuffer;
		uint32_t indexCount;

		VkDescriptorSet globalDescriptorSet;

		VkDescriptorSet dynamicDescriptorSet;
		uint32_t		dynamicOffset;

		VkDescriptorSet samplerDescriptorSet;
		VkDescriptorSet pointLightDescriptorSet;
	};

	class DrawStream
	{
	public:
		DrawStream();
		~DrawStream();

		void Draw(const DrawCommand& command);

		DrawCommand&	   operator[](std::uint32_t index);
		const DrawCommand& operator[](std::uint32_t index) const;

		[[nodiscard]] std::uint32_t Size() const;

	private:
		std::vector<DrawCommand> m_commands;
	};
}
