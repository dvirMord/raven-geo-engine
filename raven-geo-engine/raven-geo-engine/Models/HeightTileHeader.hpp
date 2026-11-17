#pragma once
#include <vector>
using std::vector;

namespace Models
{
	struct NorthWestCorner
	{
		double latitudeDegrees = 0.0;
		double longitudeDegrees = 0.0;
	};

	struct HeightTileHeader
	{
		//rows go south, columns go east
		NorthWestCorner northWestCorner;
		double sizeOfCellLatDegrees = 0.0;
		double sizeOfCellLonDegrees = 0.0;
		size_t numCellsRows = 0;
		size_t numCellsColumns = 0;
		size_t numBlocksRows = 0;
		size_t numBlocksColumns = 0;
		vector<float> blockMaxHeights;
		vector<float> heightData;
	};
}