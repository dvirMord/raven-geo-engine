#include "CellRayMarcher.hpp"
#include "../Math/GeoConversions.hpp"
#include <limits>
#include <algorithm>

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

		//second step: get the how much the ray is moving in degrees in latitude and longitude for each meter of movement in the ray direction
		Math::DegreesOffset degreesPerMeter = Math::metersToDegrees(ray.direction.getX(), ray.direction.getY(), ray.origin.getX());

		//the start point of the ray march
		optional<TerrainCell> cell = this->_heightProvider.getCellAt(ray.origin.getX(), ray.origin.getY());
		if(cell == std::nullopt)
		{
			result.hitStatus = RayHitStatus::NoTerrainData;
			return result;
		}	

		double t = 0.0;

		double tExit = this->metersToCellExit(cell.value(), ray, degreesPerMeter);


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
}