//
// Created by kakesinfo on 27/11/2025.
//

#pragma once

namespace BetterThanNothing
{
	class VulkanPipelineBuilder;
	struct MeshData;

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

		virtual void AddPipeline(const std::function<void(IPipelineBuilder*)>& callback) = 0;
    };
};
