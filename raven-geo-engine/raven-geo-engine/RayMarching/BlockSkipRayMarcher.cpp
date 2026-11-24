#include "BlockSkipRayMarcher.hpp"
#include "../Math/GeoConversions.hpp"
#include "../Common/Constants.hpp"
#include "../Models/GroundIntersection.hpp"
#include <spdlog/spdlog.h>
#include <limits>
#include <algorithm>

using Models::GroundIntersection;
using Models::TerrainBlock;

namespace RayMarching
{
	BlockSkipRayMarcher::BlockSkipRayMarcher(const IHeightProvider& heightProvider): 
		_heightProvider(heightProvider){}
	
	RayHitResult BlockSkipRayMarcher::getFirstHit(const Ray& ray) const
	{
		RayHitResult result;
		//first step: check if the ray is pointing downwards, if not return miss
		if (ray.direction.getZ() <= 0)
		{
			result.hitStatus = RayHitStatus::Miss;
			return result;	
		}

		//get the how much the ray is moving in degrees in latitude and longitude for each meter of movement in the ray direction
		Math::DegreesOffset degreesPerMeter = Math::metersToDegrees(ray.direction.getX(), ray.direction.getY(), ray.origin.getX());

		double t = 0.0;
		optional<double>  hit = std::nullopt;
		GroundIntersection point;
		
		do
		{
			if (t > Constants::RayMarching::MAX_RAY_DISTANCE_METERS)
			{
				result.hitStatus = RayHitStatus::Miss;
				return result;
			}
			double raylat = ray.origin.getX() +	(t + Constants::RayMarching::NEXT_CELL_NUDGE_METERS) * degreesPerMeter.latitudeDegrees;
			double raylon = ray.origin.getY() + (t + Constants::RayMarching::NEXT_CELL_NUDGE_METERS) * degreesPerMeter.longitudeDegrees;
			
			optional<double> skipTo = metersToBlockSkip(raylat, raylon, ray, degreesPerMeter);
			if (skipTo)
			{
				t = skipTo.value();
				continue;
			}
			
			//the current cell in the loop
			optional<TerrainCell> cell = this->_heightProvider.getCellAt(raylat, raylon);
			if (cell == std::nullopt)
			{
				result.hitStatus = RayHitStatus::NoTerrainData;
				return result;
			}

			double tExit = this->metersToExit(cell.value().bounds, ray, degreesPerMeter);

			if (t >= tExit)
			{
				spdlog::warn(Constants::Logs::RAY_MARCH_STUCK, tExit, t);
				result.hitStatus = RayHitStatus::Miss;
				return result;
			}

			hit = metersToCellHit(cell.value(), ray, t, tExit);
			if (hit != std::nullopt)
			{
				point.latitude = ray.origin.getX() + hit.value() * degreesPerMeter.latitudeDegrees;
				point.longitude = ray.origin.getY() + hit.value() * degreesPerMeter.longitudeDegrees;
				point.altitudeMeters = ray.origin.getZ() - hit.value() * ray.direction.getZ();
				point.slantRangeMeters = hit.value();
				break;
			}
			t = tExit;
		} while (!hit);

		result.hitStatus = RayHitStatus::Hit;
		result.intersectionPoint = point;
		return result;
	}

	double BlockSkipRayMarcher::metersToExit(const Bounds& bounds, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const
	{
		//third step : which wall the ray faces
		bool goingNorth = ray.direction.getX() > 0;
		bool goingEast = ray.direction.getY() > 0;

		double metersToNorthSouthWall = std::numeric_limits<double>::infinity();
		double metersToEastWestWall = std::numeric_limits<double>::infinity();
		if (degreesPerMeter.latitudeDegrees != 0)
		{
			if (goingNorth)
			{
				double degreesToWall = bounds.northLatitudeDegrees - ray.origin.getX();
				metersToNorthSouthWall = degreesToWall / degreesPerMeter.latitudeDegrees;
			}
			else
			{
				double degreesToWall = bounds.southLatitudeDegrees - ray.origin.getX();
				metersToNorthSouthWall = degreesToWall / degreesPerMeter.latitudeDegrees;
			}
		}
		if (degreesPerMeter.longitudeDegrees != 0)
		{
			if (goingEast)
			{
				double degreesToWall = bounds.eastLongitudeDegrees - ray.origin.getY();
				metersToEastWestWall = degreesToWall / degreesPerMeter.longitudeDegrees;
			}
			else
			{
				double degreesToWall =	bounds.westLongitudeDegrees - ray.origin.getY();
				metersToEastWestWall = degreesToWall / degreesPerMeter.longitudeDegrees;
			}
		}
		return std::min(metersToNorthSouthWall, metersToEastWestWall);
	}

	optional<double> BlockSkipRayMarcher::metersToCellHit(const TerrainCell& cell, const Ray& ray, const double tEntry, const double tExit) const
	{
		//first case: the ray hit the cell like a big wall, when high at tEntry is lower than the cell height
		double entryHeight = ray.origin.getZ() - tEntry * ray.direction.getZ();
		if(entryHeight < cell.heightMeters)
		{
			return tEntry;
		}

		//second case: the ray enters above the cell and goes below its height before it leaves - it hits the top (floor)
		double exitHeight = ray.origin.getZ() - tExit * ray.direction.getZ();
		if (cell.heightMeters > exitHeight)
		{
			return tEntry + (entryHeight - cell.heightMeters) / ray.direction.getZ();
		}

		//third case: the ray passed the cell without hitting it
		return std::nullopt;
	}

	optional<double> BlockSkipRayMarcher::metersToBlockSkip(const double latitude, const double longitude, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const
	{
		optional<TerrainBlock> block = this->_heightProvider.getBlockAt(latitude,longitude);
		if (!block)
		{
			// no block here means no tile - can't skip, and the cell lookup right after returns NoTerrainData
			return std::nullopt; 
		}
		double blockExit = this->metersToExit(block.value().bounds, ray, degreesPerMeter);

		double exitHeight = ray.origin.getZ() - blockExit * ray.direction.getZ();
		if (exitHeight > block.value().maxHeightMeters)
		{
			return blockExit;
		}
		return std::nullopt;
	}
}