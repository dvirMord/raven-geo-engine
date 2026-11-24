#include "CellRayMarcher.hpp"
#include "../Math/GeoConversions.hpp"
#include "../Common/Constants.hpp"
#include "../Models/GroundIntersection.hpp"
#include <spdlog/spdlog.h>
#include <limits>
#include <algorithm>

using Models::GroundIntersection;

namespace RayMarching
{
	CellRayMarcher::CellRayMarcher(const IHeightProvider& heightProvider): 
		_heightProvider(heightProvider){}
	
	RayHitResult CellRayMarcher::getFirstHit(const Ray& ray) const
	{
		RayHitResult result;
		//first step: check if the eay is pointing downwards, if not return miss
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
			double raylat = ray.origin.getX() +	(t + Constants::RayMarching::NEXT_CELL_NUDGE_METERS) * degreesPerMeter.latitudeDegrees;
			double raylon = ray.origin.getY() + (t + Constants::RayMarching::NEXT_CELL_NUDGE_METERS) * degreesPerMeter.longitudeDegrees;
			//the current cell in the loop
			optional<TerrainCell> cell = this->_heightProvider.getCellAt(raylat, raylon);
			if (cell == std::nullopt)
			{
				result.hitStatus = RayHitStatus::NoTerrainData;
				return result;
			}

			double tExit = this->metersToCellExit(cell.value(), ray, degreesPerMeter);

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
			
			if (t > Constants::RayMarching::MAX_RAY_DISTANCE_METERS)
			{
				spdlog::info(Constants::Logs::RAY_MARCH_MISS, Constants::RayMarching::MAX_RAY_DISTANCE_METERS);
				result.hitStatus = RayHitStatus::Miss;
				return result;
			}
		} while (!hit);

		result.hitStatus = RayHitStatus::Hit;
		result.intersectionPoint = point;
		return result;
	}

	double CellRayMarcher::metersToCellExit(const TerrainCell& cell, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const
	{
		//third step : which wall the ray faces
		bool goingNorth = ray.direction.getX() > 0;
		bool goingEast = ray.direction.getY() > 0;

		double metersToNextCellNorth = std::numeric_limits<double>::infinity();
		double metersToNextCellEast = std::numeric_limits<double>::infinity();
		if (degreesPerMeter.latitudeDegrees != 0)
		{
			if (goingNorth)
			{
				double degreesToWall = cell.bounds.northLatitudeDegrees - ray.origin.getX();
				metersToNextCellNorth = degreesToWall / degreesPerMeter.latitudeDegrees;
			}
			else
			{
				double degreesToWall = cell.bounds.southLatitudeDegrees - ray.origin.getX();
				metersToNextCellNorth = degreesToWall / degreesPerMeter.latitudeDegrees;
			}
		}
		if (degreesPerMeter.longitudeDegrees != 0)
		{
			if (goingEast)
			{
				double degreesToWall = cell.bounds.eastLongitudeDegrees - ray.origin.getY();
				metersToNextCellEast = degreesToWall / degreesPerMeter.longitudeDegrees;
			}
			else
			{
				double degreesToWall = cell.bounds.westLongitudeDegrees - ray.origin.getY();
				metersToNextCellEast = degreesToWall / degreesPerMeter.longitudeDegrees;
			}
		}
		return std::min(metersToNextCellNorth, metersToNextCellEast);
	}

	optional<double> CellRayMarcher::metersToCellHit(const TerrainCell& cell, const Ray& ray, const double tEntry, const double tExit) const
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

		//third case: the ray passsed the cell without hitting it
		return std::nullopt;
	}

}