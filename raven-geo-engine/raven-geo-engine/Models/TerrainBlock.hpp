#pragma once
#include "Bounds.hpp"

namespace Models
{
	struct TerrainBlock
	{
		double maxHeightMeters = 0.0;
		Bounds bounds = {};
	};
}