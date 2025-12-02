//
// Created by COEZARAX on 02/12/2025.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class IPipelineBuilder
	{
	public:
		virtual ~IPipelineBuilder() = default;

		virtual IPipelineBuilder& SetName(const char* name) = 0;
		virtual IPipelineBuilder& AddVertexShader(const std::string& shaderPath) = 0;
		virtual IPipelineBuilder& AddFragmentShader(const std::string& shaderPath) = 0;
	};
} // BetterThanNothing