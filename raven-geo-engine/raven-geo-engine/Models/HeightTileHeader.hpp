#pragma once
#include <vector>
using std::vector;

namespace Models
{
	// the north-west corner of the tile - the highest latitude and the most western longitude.
	// row 0 / column 0 starts here: rows go south, columns go east
	struct NorthWestCorner
	{
		double latitudeDegrees = 0.0;
		double longitudeDegrees = 0.0;
	};

	struct HeightTileHeader
	{
		NorthWestCorner northWestCorner;
		double sizeOfCellLatDegrees = 0.0;
		double sizeOfCellLonDegrees = 0.0;
		size_t numCellsRows = 0;
		size_t numCellsColumns = 0;
		vector<float> heightData;
	};
}