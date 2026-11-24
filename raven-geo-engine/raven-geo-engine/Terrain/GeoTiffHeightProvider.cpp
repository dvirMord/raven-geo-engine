#include "GeoTiffHeightProvider.hpp"
#include "../Common/Constants.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <spdlog/spdlog.h>
#include <tiffio.h>
#include <limits>

using std::nullopt;
using std::optional;
using Constants::GeoTiff::BLOCK_SIZE;

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
			size_t chunkWidth = 0;   // the file is split into chunks of this size (1024x1024 for Copernicus)
			size_t chunkHeight = 0;
		};

		// read the grid size and chunk size - nullopt if the file isn't a grid of 32 bit floats split into chunks
		optional<GridLayout> readGridLayout(TIFF* tiff)
		{
			uint32_t numColumns = 0;
			uint32_t numRows = 0;
			uint32_t chunkWidth = 0;
			uint32_t chunkHeight = 0;
			TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &numColumns);
			TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &numRows);
			TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &chunkWidth);
			TIFFGetField(tiff, TIFFTAG_TILELENGTH, &chunkHeight);

			uint16_t bitsPerSample = 0;
			uint16_t sampleFormat = 0;
			TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
			TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat);

			// readHeights reads chunk by chunk straight into floats - any other layout would loop forever or give garbage heights
			if (!TIFFIsTiled(tiff) || chunkWidth == 0 || chunkHeight == 0 || numColumns == 0 || numRows == 0
				|| bitsPerSample != Constants::GeoTiff::HEIGHT_BITS_PER_SAMPLE || sampleFormat != SAMPLEFORMAT_IEEEFP)
			{
				return nullopt;
			}
			return GridLayout{ numRows, numColumns, chunkWidth, chunkHeight };
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
			tileHeader.northWestCorner.latitudeDegrees = tie[Constants::GeoTiff::CORNER_LAT_INDEX];
			tileHeader.northWestCorner.longitudeDegrees = tie[Constants::GeoTiff::CORNER_LON_INDEX];
			tileHeader.sizeOfCellLatDegrees = scale[Constants::GeoTiff::CELL_SIZE_LAT_INDEX];
			tileHeader.sizeOfCellLonDegrees = scale[Constants::GeoTiff::CELL_SIZE_LON_INDEX];
			return true;
		}

		// copy one unpacked chunk into its place in the big grid
		void copyChunkIntoGrid(const vector<float>& chunk, size_t chunkTopRow, size_t chunkLeftColumn,
			const GridLayout& layout, vector<float>& heights)
		{
			// chunks on the right and bottom edges stick out past the grid - copy only the part inside it
			size_t rowsInside = std::min(layout.chunkHeight, layout.numRows - chunkTopRow);
			size_t columnsInside = std::min(layout.chunkWidth, layout.numColumns - chunkLeftColumn);

			for (size_t rowInChunk = 0; rowInChunk < rowsInside; ++rowInChunk)
			{
				// both are stored row after row, so a row starts at: row * rowLength + column
				size_t chunkRowStart = rowInChunk * layout.chunkWidth;
				size_t gridRowStart = (chunkTopRow + rowInChunk) * layout.numColumns + chunkLeftColumn;

				std::copy_n(chunk.begin() + chunkRowStart, columnsInside, heights.begin() + gridRowStart);
			}
		}

		// unpack every chunk of the file into one flat grid of heights - false if a chunk can't be read
		bool readHeights(TIFF* tiff, const GridLayout& layout, vector<float>& heights)
		{
			heights.resize(layout.numRows * layout.numColumns);
			vector<float> chunk(layout.chunkWidth * layout.chunkHeight);

			for (size_t chunkTopRow = 0; chunkTopRow < layout.numRows; chunkTopRow += layout.chunkHeight)
			{
				for (size_t chunkLeftColumn = 0; chunkLeftColumn < layout.numColumns; chunkLeftColumn += layout.chunkWidth)
				{
					// libtiff finds the chunk that holds this cell and unzips it into the buffer
					tmsize_t readBytes = TIFFReadTile(tiff, chunk.data(),
						static_cast<uint32_t>(chunkLeftColumn), static_cast<uint32_t>(chunkTopRow), 0, 0);
					if (readBytes < 0)
					{
						return false;
					}

					copyChunkIntoGrid(chunk, chunkTopRow, chunkLeftColumn, layout, heights);
				}
			}
			return true;
		}

		void buildBlockMaxTable(Models::HeightTileHeader& tileHeader)
		{
			//the size of the block table
			size_t numBlocksRows = (tileHeader.numCellsRows + BLOCK_SIZE - 1) / BLOCK_SIZE;
			size_t numBlockCols = (tileHeader.numCellsColumns + BLOCK_SIZE - 1) / BLOCK_SIZE;
			tileHeader.numBlocksRows = numBlocksRows;
			tileHeader.numBlocksColumns = numBlockCols;

			//resize the block to the tiles size, init the lowest number in it 
			tileHeader.blockMaxHeights.assign(numBlocksRows * numBlockCols, std::numeric_limits<float>::lowest());

			for (size_t row = 0; row < tileHeader.numCellsRows; row++)
			{
				for (size_t colum = 0; colum < tileHeader.numCellsColumns; colum++)
				{
					float height = tileHeader.heightData[row * tileHeader.numCellsColumns + colum];
					size_t blockRow = row / BLOCK_SIZE;
					size_t blockCol = colum / BLOCK_SIZE;
					size_t indexInTable = blockRow * numBlockCols + blockCol;

					if (tileHeader.blockMaxHeights[indexInTable] < height)
					{
						tileHeader.blockMaxHeights[indexInTable] = height;
					}
				}
			}
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
		buildBlockMaxTable(tileHeader);
		return tileHeader;
	}

	optional<GeoTiffHeightProvider::CellLocation> GeoTiffHeightProvider::findCell(const double latitude, const double longitude) const
	{
		for (const auto& tile : this->_tiles)
		{
			double latitudeInrow = (tile.northWestCorner.latitudeDegrees - latitude) / tile.sizeOfCellLatDegrees;
			double longitudeIncolumn = (longitude - tile.northWestCorner.longitudeDegrees) / tile.sizeOfCellLonDegrees;

			if (latitudeInrow < 0 || longitudeIncolumn < 0)
			{
				continue; // the point is north or west of the tile
			}

			size_t row = static_cast<size_t>(latitudeInrow);
			size_t col = static_cast<size_t>(longitudeIncolumn);

			if (row >= tile.numCellsRows || col >= tile.numCellsColumns)
			{
				continue; // the point is south or east of the tile
			}

			return CellLocation{ &tile, row, col };
		}
		return nullopt; // no tile covers the point
	}

	optional<TerrainCell> GeoTiffHeightProvider::getCellAt(const double latitude, const double longitude) const
	{
		optional<CellLocation> location = this->findCell(latitude, longitude);
		if (!location)
		{
			return nullopt;
		}
		const Models::HeightTileHeader& tile = *location->tile;
		size_t row = location->row;
		size_t col = location->col;

		TerrainCell cell;
		cell.heightMeters = tile.heightData[row * tile.numCellsColumns + col];
		// rows go south from the corner (latitude shrinks), columns go east (longitude grows)
		cell.bounds.northLatitudeDegrees = tile.northWestCorner.latitudeDegrees - (tile.sizeOfCellLatDegrees * row);
		cell.bounds.southLatitudeDegrees = cell.bounds.northLatitudeDegrees - tile.sizeOfCellLatDegrees;
		cell.bounds.westLongitudeDegrees = tile.northWestCorner.longitudeDegrees + (tile.sizeOfCellLonDegrees * col);
		cell.bounds.eastLongitudeDegrees = cell.bounds.westLongitudeDegrees + tile.sizeOfCellLonDegrees;
		return cell;
	}

	optional<TerrainBlock> GeoTiffHeightProvider::getBlockAt(const double latitude, const double longitude) const
	{
		optional<CellLocation> location = this->findCell(latitude, longitude);
		if (!location)
		{
			return nullopt;
		}
		const Models::HeightTileHeader& tile = *location->tile;
		size_t blockRow = location->row / BLOCK_SIZE;
		size_t blockCol = location->col / BLOCK_SIZE;

		double blockLatSize = tile.sizeOfCellLatDegrees * BLOCK_SIZE;
		double blockLonSize = tile.sizeOfCellLonDegrees * BLOCK_SIZE;
		TerrainBlock block;
		block.maxHeightMeters = tile.blockMaxHeights[blockRow * tile.numBlocksColumns + blockCol];
		// rows go south from the corner (latitude shrinks), columns go east (longitude grows)
		block.bounds.northLatitudeDegrees = tile.northWestCorner.latitudeDegrees - (blockLatSize * blockRow);
		block.bounds.southLatitudeDegrees = block.bounds.northLatitudeDegrees - blockLatSize;
		block.bounds.westLongitudeDegrees = tile.northWestCorner.longitudeDegrees + (blockLonSize * blockCol);
		block.bounds.eastLongitudeDegrees = block.bounds.westLongitudeDegrees + blockLonSize;
		return block;
	}
}