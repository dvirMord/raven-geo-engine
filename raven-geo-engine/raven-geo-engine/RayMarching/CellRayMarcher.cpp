#include "CellRayMarcher.hpp"
#include "../Math/GeoConversions.hpp"

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
		Math::DegreesOffset degreesOffset = Math::metersToDegrees(ray.direction.getX(), ray.direction.getY(), ray.origin.getX());

		return result;
	}
}