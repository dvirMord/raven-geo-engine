#pragma once
#include "IHeightProvider.hpp"
#include "../Models/HeightTileHeader.hpp"
#include "../Models/TerrainCell.hpp"
#include "../Models/TerrainBlock.hpp"
#include <string>
#include <vector>
#include <optional>
using Models::TerrainBlock;
using Models::TerrainCell;
using std::string;
using std::vector;
using std::optional;


namespace Terrain
{
	class GeoTiffHeightProvider : public IHeightProvider
	{
	public:
		explicit GeoTiffHeightProvider(const vector<string>& tilesPaths);
		~GeoTiffHeightProvider() override = default;

		optional<TerrainCell> getCellAt(const double latitude, const double longitude) const override;
		optional<TerrainBlock> getBlockAt(const double latitude, const double longitude) const override;

	private:
		// where a point falls: which tile, and which cell (row, column) inside it
		struct CellLocation
		{
			const Models::HeightTileHeader* tile; // a pointer because optional can't hold a reference - safe, _tiles never changes after the constructor
			size_t row;
			size_t col;
		};

		vector<Models::HeightTileHeader> _tiles;
		optional<Models::HeightTileHeader> loadTile(const string& path);

		// the tile and cell under the point - nullopt when no loaded tile covers it
		optional<CellLocation> findCell(const double latitude, const double longitude) const;
	};
}