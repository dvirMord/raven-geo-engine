#pragma once
#include <optional>
#include "../Models/TerrainCell.hpp"
#include "../Models/TerrainBlock.hpp"
using Models::TerrainCell;
using Models::TerrainBlock;
using std::optional;

namespace Terrain
{
	class IHeightProvider
	{
	public:
		virtual ~IHeightProvider() = default;
		// the cell at the point: height in meters above sea level, bounds in degrees - nullopt when no loaded tile covers it
		virtual optional<TerrainCell> getCellAt(const double latitude, const double longitude) const = 0;
		// the block at the point: the highest cell height in it (meters), bounds in degrees - nullopt when no loaded tile covers it
		virtual optional<TerrainBlock> getBlockAt(const double latitude, const double longitude) const = 0;
	};
}	