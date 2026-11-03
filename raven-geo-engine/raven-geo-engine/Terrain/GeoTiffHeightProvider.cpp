#include "GeoTiffHeightProvider.hpp"
#include "../Common/Constants.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <spdlog/spdlog.h>
#include <tiffio.h>

using std::nullopt;
using std::optional;

namespace Terrain
{
	// helpers used only by loadTile - they live here so libtiff stays out of the header
	namespace
	{
		// how the grid is laid out inside the file
		struct GridLayout
		{
			size_t numRows = 0;
			size_t numColumns = 0;
			size_t blockWidth = 0;   // the file is split into blocks of this size (1024x1024 for Copernicus)
			size_t blockHeight = 0;
		};

		// read the grid size and block size - nullopt if the file isn't a grid of 32 bit floats split into blocks
		optional<GridLayout> readGridLayout(TIFF* tiff)
		{
			uint32_t numColumns = 0;
			uint32_t numRows = 0;
			uint32_t blockWidth = 0;
			uint32_t blockHeight = 0;
			TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &numColumns);
			TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &numRows);
			TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &blockWidth);
			TIFFGetField(tiff, TIFFTAG_TILELENGTH, &blockHeight);

			uint16_t bitsPerSample = 0;
			uint16_t sampleFormat = 0;
			TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
			TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat);

			// readHeights reads block by block straight into floats - any other layout would loop forever or give garbage heights
			if (!TIFFIsTiled(tiff) || blockWidth == 0 || blockHeight == 0 || numColumns == 0 || numRows == 0
				|| bitsPerSample != Constants::GeoTiff::HEIGHT_BITS_PER_SAMPLE || sampleFormat != SAMPLEFORMAT_IEEEFP)
			{
				return nullopt;
			}
			return GridLayout{ numRows, numColumns, blockWidth, blockHeight };
		}

		// read where the grid sits on earth (corner + cell size) into the tile - false if the tags are missing or broken
		bool readGeoReference(TIFF* tiff, Models::HeightTileHeader& tileHeader)
		{
			uint32_t scaleCount = 0; // libtiff writes 4 bytes here for these two tags - a uint16_t would be overrun
			double* scale = nullptr;
			TIFFGetField(tiff, Constants::GeoTiff::CELL_SIZE_TAG, &scaleCount, &scale);

			uint32_t tieCount = 0;
			double* tie = nullptr;
			TIFFGetField(tiff, Constants::GeoTiff::CORNER_TAG, &tieCount, &tie);

			if (tieCount < Constants::GeoTiff::CORNER_MIN_COUNT || scaleCount < Constants::GeoTiff::CELL_SIZE_MIN_COUNT
				|| scale == nullptr || tie == nullptr
				|| scale[Constants::GeoTiff::CELL_SIZE_LAT_INDEX] <= 0 || scale[Constants::GeoTiff::CELL_SIZE_LON_INDEX] <= 0)
			{
				return false;
			}

			// copy the values now - scale and tie belong to libtiff and are freed when the file closes
			tileHeader.northWestCorner.latitude = tie[Constants::GeoTiff::CORNER_LAT_INDEX];
			tileHeader.northWestCorner.longitude = tie[Constants::GeoTiff::CORNER_LON_INDEX];
			tileHeader.sizeOfCellLatDegrees = scale[Constants::GeoTiff::CELL_SIZE_LAT_INDEX];
			tileHeader.sizeOfCellLonDegrees = scale[Constants::GeoTiff::CELL_SIZE_LON_INDEX];
			return true;
		}

		// copy one unpacked block into its place in the big grid
		void copyBlockIntoGrid(const vector<float>& block, size_t blockTopRow, size_t blockLeftColumn,
			const GridLayout& layout, vector<float>& heights)
		{
			// blocks on the right and bottom edges stick out past the grid - copy only the part inside it
			size_t rowsInside = std::min(layout.blockHeight, layout.numRows - blockTopRow);
			size_t columnsInside = std::min(layout.blockWidth, layout.numColumns - blockLeftColumn);

			for (size_t rowInBlock = 0; rowInBlock < rowsInside; ++rowInBlock)
			{
				// both are stored row after row, so a row starts at: row * rowLength + column
				size_t blockRowStart = rowInBlock * layout.blockWidth;
				size_t gridRowStart = (blockTopRow + rowInBlock) * layout.numColumns + blockLeftColumn;

				std::copy_n(block.begin() + blockRowStart, columnsInside, heights.begin() + gridRowStart);
			}
		}

		// unpack every block of the file into one flat grid of heights - false if a block can't be read
		bool readHeights(TIFF* tiff, const GridLayout& layout, vector<float>& heights)
		{
			heights.resize(layout.numRows * layout.numColumns);
			vector<float> block(layout.blockWidth * layout.blockHeight);

			for (size_t blockTopRow = 0; blockTopRow < layout.numRows; blockTopRow += layout.blockHeight)
			{
				for (size_t blockLeftColumn = 0; blockLeftColumn < layout.numColumns; blockLeftColumn += layout.blockWidth)
				{
					// libtiff finds the block that holds this cell and unzips it into the buffer
					tmsize_t readBytes = TIFFReadTile(tiff, block.data(),
						static_cast<uint32_t>(blockLeftColumn), static_cast<uint32_t>(blockTopRow), 0, 0);
					if (readBytes < 0)
					{
						return false;
					}

					copyBlockIntoGrid(block, blockTopRow, blockLeftColumn, layout, heights);
				}
			}
			return true;
		}
	}

	GeoTiffHeightProvider::GeoTiffHeightProvider(const vector<string>& tilesPaths)
	{
		for (const auto& tilePath : tilesPaths)
		{
			optional<Models::HeightTileHeader> tileHeader = loadTile(tilePath);
			if (tileHeader)
			{
				// move, not copy - a tile holds ~13M heights
				this->_tiles.push_back(std::move(*tileHeader));
				spdlog::info(Constants::Logs::TILE_LOADED, tilePath);
			}
			else
			{
				// a missing tile only affects its own region, so keep loading the rest
				spdlog::warn(Constants::Logs::TILE_LOAD_FAILED, tilePath);
			}
		}

		spdlog::info(Constants::Logs::TILES_LOADED_SUMMARY, this->_tiles.size(), tilesPaths.size());
		if (this->_tiles.empty())
		{
			spdlog::error(Constants::Logs::NO_TILES_LOADED);
		}
	}

	optional<Models::HeightTileHeader> GeoTiffHeightProvider::loadTile(const string& tilePath)
	{
		// the file closes by itself on every return below
		std::unique_ptr<TIFF, decltype(&TIFFClose)> tiff(TIFFOpen(tilePath.c_str(), "r"), &TIFFClose);
		if (tiff == nullptr)
		{
			return nullopt;
		}

		optional<GridLayout> layout = readGridLayout(tiff.get());
		if (!layout)
		{
			return nullopt;
		}

		Models::HeightTileHeader tileHeader;
		if (!readGeoReference(tiff.get(), tileHeader))
		{
			return nullopt;
		}
		tileHeader.numCellsRows = layout->numRows;
		tileHeader.numCellsColumns = layout->numColumns;

		if (!readHeights(tiff.get(), *layout, tileHeader.heightData))
		{
			return nullopt;
		}

		return tileHeader;
	}

	optional<double> GeoTiffHeightProvider::getHeightMeters(const double latitude, const double longitude) const
	{
		for (const auto& tile : this->_tiles)
		{
			double latitudeInrow = (tile.northWestCorner.latitude - latitude) / tile.sizeOfCellLatDegrees;
			double longitudeIncolumn = (longitude - tile.northWestCorner.longitude) / tile.sizeOfCellLonDegrees;

			if (latitudeInrow < 0 || longitudeIncolumn < 0)
			{
				continue;  // the point is north or east of the tile
			}

			size_t row = static_cast<size_t>(latitudeInrow);
			size_t col = static_cast<size_t>(longitudeIncolumn);	

			if(row >= tile.numCellsRows || col >= tile.numCellsColumns)
			{
				continue; // the point is north or east of the tile
			}

			return tile.heightData[row * tile.numCellsColumns + col];
		}
		return nullopt; // no tile covers the point
	}
}