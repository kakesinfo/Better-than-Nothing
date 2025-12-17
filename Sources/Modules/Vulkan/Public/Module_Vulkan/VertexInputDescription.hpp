#pragma once

namespace BetterThanNothing
{
	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription>	bindings;
		std::vector<VkVertexInputAttributeDescription>	attributes;

		VkPipelineVertexInputStateCreateInfo GetCreateInfo() const
		{
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			vertexInputInfo.vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindings.size());
			vertexInputInfo.pVertexBindingDescriptions = bindings.data();

			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
			vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

			return vertexInputInfo;
		}
	};
}