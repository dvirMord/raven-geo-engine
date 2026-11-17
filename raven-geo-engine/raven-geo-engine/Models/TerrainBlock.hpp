#pragma once
#include "Bounds.hpp"

namespace Models
{
	struct TerrainBlock
	{
		float maxHeightMeters = 0.0;
		Bounds bounds = {};
	};
}