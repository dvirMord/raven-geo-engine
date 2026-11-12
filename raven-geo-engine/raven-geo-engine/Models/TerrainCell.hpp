#pragma once
#include "Bounds.hpp"

namespace Models
{
	struct TerrainCell
	{
		double heightMeters = 0.0;
		Bounds bounds = {};
	};
}