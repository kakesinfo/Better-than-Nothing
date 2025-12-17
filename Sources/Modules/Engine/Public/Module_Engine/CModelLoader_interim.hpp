#pragma once

namespace BetterThanNothing
{
	struct ModelData;

	class CModelLoader_interim
	{
	public:
		static ModelData LoadModel(const std::string& path, bool preTransform = true);
	};
}