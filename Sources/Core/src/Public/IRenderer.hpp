//
// Created by COEZARAX on 27/11/2025.
//

#pragma once

namespace BetterThanNothing
{
	class VulkanPipelineBuilder;
	class CMesh;

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

		virtual void AddPipeline(const std::function<void(VulkanPipelineBuilder&)>& callback) = 0;
    	virtual void Render(CMesh* pMesh) = 0;
    };
};