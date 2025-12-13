#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CPlaneMesh: public MeshData
	{
	public:
		explicit CPlaneMesh(float32 size = 1.0f);

	private:
		void _generatePlane(float32 size);
	};
}