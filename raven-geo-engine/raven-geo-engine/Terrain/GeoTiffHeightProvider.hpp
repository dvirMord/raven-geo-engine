#pragma once
#include "IHeightProvider.hpp"
#include "../Models/HeightTileHeader.hpp"
#include <string>
#include <vector>
#include <optional>
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

		optional<double> getHeightMeters(const double latitude, const double longitude) const override;

	private:
		vector<Models::HeightTileHeader> _tiles;
		optional<Models::HeightTileHeader> loadTile(const string& path);
	};
}