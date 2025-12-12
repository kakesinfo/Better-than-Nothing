#pragma once

namespace BetterThanNothing
{
	class CModelLoader_interim
	{
	public:
		static ModelData LoadModel(const std::string& path, bool preTransform = true);
	};
} // BetterThanNothing