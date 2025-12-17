#pragma once

#include <Module_Data/Graphics/MeshData.hpp>

namespace BetterThanNothing
{
	class CPlaneMesh: public MeshData
	{
	public:
		explicit CPlaneMesh(float size = 1.0f);

	private:
		void _generatePlane(float size);
	};
}