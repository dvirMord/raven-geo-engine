#pragma once
#include <optional>
#include "../Models/TerrainCell.hpp"
using Models::TerrainCell;
using std::optional;

namespace Terrain
{
	class IHeightProvider
	{
	public:
		virtual ~IHeightProvider() = default;
		// the cell at the point: height in meters above sea level, bounds in degrees - nullopt when no loaded tile covers it
		virtual optional<TerrainCell> getCellAt(const double latitude, const double longitude) const = 0;
	};
}